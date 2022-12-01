// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

const MAX_TRY_SCAN = 1000;

function sleep(ms)
{
    return new Promise(resolve => setTimeout(resolve, ms));
}

function status_to_state (code)
{
    if( code == 200 ) { return "FINISHED"; }
    if( code == 102 ) { return "PENDING";  }
    if( code >= 400 ) { return "FAILED";   }
    
    return "UNKNOWN";
}

function process_result (r)
{
    let status = { "state"  : "NULL",
                   "result" : "" ,
                   "message": "" };
    //console.log("=== " + r);
    status.result  = JSON.parse(r);
    status.message = status.result.message;
    status.state   = status_to_state( status.result.status );
    return status;
}

function process_error (e)
{
    let status = { "state"  : "NULL",
                   "result" : "" ,
                   "message": "" };
    //console.log("--- " + e);
    status.message = e;
    status.state   = "FAILED";
    return status;
}

async function trigger(url)
{
    let err_msg = ""
    let result_json = ""

    try
    {
        let response = await fetch(url, { mode: "no-cors" })
        if (response.ok)
        {
            let result_text = await response.text();
            return Promise.resolve(result_text);
        }
        else
        {
            err_msg = response.statusText;
        }
    }
    catch (err)
    {
        err_msg = err.message;
    }

    return Promise.reject(err_msg);
}

class Backend
{
    constructor(server) { this.server = server; }
    
    os_status (uuid)
    {
        let url = this.server + '/os-status';
        if (uuid) { url += "?uuid=" + uuid; }

        return trigger (url)
            .then( (result) => { return process_result(result); },
                   (err)    => { return process_error(err);     });
    }

    factory_reset (uuid)
    {
        let url = this.server + "/factory-reset?scanuuid=" + uuid;

        return trigger(url)
            .then( (result) => { return process_result(result); },
                   (err)    => { return process_error(err);     });        
    }

    undo_reset ()
    {
        let url = this.server + "/undo-reset";

        return trigger(url)
            .then( (result) => { return process_result(result); },
                   (err)    => { return process_error(err);     });        
    }

    get_status_by_uuid (uuid, start, max)
    {
        let url = this.server + "/status" + "?uuid=" + uuid;
        if (start) { url += "&start=" + start; }
        if (max  ) { url += "&max=" + max;     }

        return trigger(url)
            .then( (result) => { return process_result(result); },
                   (err)    => { return process_error(err);     });
    }

    clear_session (uuid)
    {
        let url = this.server + "/clear" + "?uuid=" + uuid;

        return trigger(url)
            .then( (result) => { return process_result(result); },
                   (err)    => { return process_error(err);     });
    }

    async poll_session (uuid, msgidx, msg_handler)
    {
        let payload  = null;
        let response = await this.get_status_by_uuid(uuid, msgidx);
        let session_data = response.result["status_list"];
        let messages = response.result.log_messages;
        
        if( session_data ) { payload = session_data[uuid]; }

        if( payload )
        {
            let code = payload[0];
            let type = payload[1];
            let text = payload[2];

            if (response.state == "FINISHED")
            {
                if (code == 200)
                {
                    if (msg_handler)
                        msg_handler(type + " complete:" + text);
                    return -1;
                }
                else if (code >= 400)
                {
                    if (msg_handler)
                        msg_handler(type + " error: " + code + ": " + text);
                    return -2;
                }
                else if (code == 102)
                {
                    let i = 0;
                    let text = '';
                    for (const line of messages)
                    {
                        text += line + "\n";
                        i++;
                    }
                    if (msg_handler && (i > 0))
                        msg_handler(text);
                    return i;
                }
                else
                {
                    if (msg_handler)
                        msg_handler( type + ": " + text );
                    return -2;
                }
            }
        }
        else
        {
            if (msg_handler)
                msg_handler("Error: " + type + " session vanished");
            return -2;
        }
    }

    async run_session (onstart, ondone, onfail, msg_handler)
    {
        let failed = false;
        const mid = 'status-log';

        let response = await onstart();

        if (response.state == "FAILED")
        {
            failed = true;
        }
        else if (response.state == "PENDING" ||
                 response.state == "FINISHED")
        {
            let count  = 0;
            let logidx = 0;
            let done   = false;

            while (count < MAX_TRY_SCAN && !failed)
            {
                await sleep(1000);
                count++;

                let session_status =
                    await this.poll_session(response.result.uuid, logidx, msg_handler);

                switch (session_status)
                {
                    case -2: failed = true; break;
                    case -1: done   = true; break
                    default:
                      if (session_status > 0) 
                          logidx += session_status;
                }

                if (done)
                {
                    ondone(response.result);
                    break;
                }
            }
        }

        if (failed)
        {
            onfail(response.result);
        }
    }
}

export { Backend };
