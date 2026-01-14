// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

const MAX_TRY_SCAN = 1000;

function status_to_state (code)
{
    if( code == 200 ) { return "FINISHED"; }
    if( code == 102 ) { return "PENDING";  }
    if( code >= 400 ) { return "FAILED";   }
    
    return "UNKNOWN";
}

function parse_json (r)
{
    let status = { "state"  : "NULL",
                   "result" : "" ,
                   "message": "" };

    try { status.result = JSON.parse(r); } catch (err) {}

    if( status.result )
    {
        status.state   = status_to_state( status.result.status );
        status.message = status.result.message;
    }
    else
    {
        status.state   = "FAILED";
        status.message = "Malformed JSON received from service: ";
        status.result  = r;
    }

    return status;
}

function get_service_response(url)
{
    let result = undefined;

    try
    {
        let req = new XMLHttpRequest();
        req.open("GET", url, false);
        req.send();

        if (req.status >= 100 && req.status < 300)
        {
            result = parse_json( req.responseText );
        }
        else if (req.status == 0)
        {
            result = { "state": "FAILED",
                       "result": {},
                       "message": "Holo Factory Reset Service Unavailable" };
        }
        else
        {
            result = { "state": "FAILED",
                       "result": req.responseText,
                       "message": req.statusText };
        }
    }
    catch (err)
    {
        result = { "state": "FAILED",
                   "result": "",
                   "message": err.message };
    }

    return result;
}

class Backend
{
    constructor (server)
    {
        this.server = server;
        this.result = undefined;
    }
    
    os_status (uuid)
    {
        let url = this.server + '/os-status';
        if (uuid) { url += "?uuid=" + uuid; }

        return get_service_response(url);
    }

    boot_status ()
    {
        let url = this.server + '/boot-status';

        return get_service_response(url);
    }
    
    factory_reset (uuid)
    {
        let url = this.server + "/factory-reset?scanuuid=" + uuid;

        return get_service_response(url);
    }

    undo_reset ()
    {
        let url = this.server + "/undo-reset";

        return get_service_response(url);
    }

    get_status_by_uuid (uuid, start, max)
    {
        let url = this.server + "/status" + "?uuid=" + uuid;
        if (start) { url += "&start=" + start; }
        if (max  ) { url += "&max=" + max;     }

        return get_service_response(url);
    }

    clear_session (uuid)
    {
        let url = this.server + "/clear" + "?uuid=" + uuid;

        return get_service_response(url)
    }

    poll_session (uuid, msgidx, msg_handler)
    {
        let payload  = null;
        let response = this.get_status_by_uuid(uuid, msgidx);
        let session_data = response.result["status_list"];
        let messages = response.result.log_messages;
        
        if( session_data ) { payload = session_data[uuid]; }

        this.result = payload;
        
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
                        msg_handler(type + " complete: " + text);
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
                msg_handler("Error: session vanished");
            console.log( response );
            return -2;
        }
    }
}
