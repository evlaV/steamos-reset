// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

import { Backend } from './backend.js';

function set_status_message (msg)
{
    let div = document.getElementById('status-log');
    if (div)
    {
        let text  = msg.replaceAll("\'", "");
        let lines = document.evaluate("//pre", div, null,
                                      XPathResult.ORDERED_NODE_SNAPSHOT_TYPE,
                                      null);

        for( let x = 0; x < lines.snapshotLength; x++ )
        {
            let logline = lines.snapshotItem(x);
            if( logline )
                div.removeChild( logline );
        }

        let logline = document.createElement( 'pre' );
        logline.textContent = text;
        div.appendChild( logline );
    }
}

function add_status_message (id,msg)
{
    let div = document.getElementById(id);

    if( div )
    {
        let text = msg.replaceAll("\'", "");
        let logline  = document.createElement( 'pre' );
        logline.textContent = text;
        div.appendChild( logline );
    }
}

function display_element (id,state)
{
    let el = false;
    if (id && (el = document.getElementById(id)))
        el.style.display = state ? "block" : "none";
}

function choose_ui_block (id)
{
    let visible = false;
    let ui = document.evaluate("//div[contains(@class,'uiblock')]",
                               document, null,
                               XPathResult.ORDERED_NODE_SNAPSHOT_TYPE,
                               null);
    if( !ui )
        return;

    for (var x = 0; x < ui.snapshotLength; x++)
    {
        let block = ui.snapshotItem(x);

        if( !block )
            continue;
 
        if (block.id == id)
        {
            block.style.display = 'block';
            visible = true;
        }
        else
        {
            block.style.display = 'none';
        }
    }

    if( !visible )
        ui.snapshotItem(0).style.display = 'block';
}

class Reset
{
    constructor ()
    {
        this.backend = new Backend("http://127.0.0.1:8080/cgi");
        this.reset_id = 0;
        this.update_needed = false;
        this.update_url = "";
    }

    async get_os_status (id)
    {
        let response = await this.backend.os_status(id);

        if (response.state == "FAILED")
        {
            add_status_message(mid, "GOS: Failed: Restart application");
        }
        else if (response.state == "PENDING")
        {
            // Shouldn't happen in normal use, as we shouldn't get around to
            // calling this until the status check has completed:
            this.reset_id = id;
        }
        else if (response.state == "FINISHED")
        {
            self.update_needed = response.result.update.needed * 1;
            self.update_url    = response.result.update.url || "";

            if (self.update_needed == 1)
            {
                let matched = self.update_url.match(/^.*\/(.+)\.\S+/);
                let image   = matched ? matched[1] : "unknown";
                set_status_message("OS Reinstall Required:" + image );
            }
            else
            {
                display_element("freshen",false);
                set_status_message("OS Up to Date");
            }

            this.reset_id = id;
            choose_ui_block("prepare");
        }
    }

    async prepare_factory_reset ()
    {
        let backend = this.backend;
        let uuid    = this.reset_id;
        let start_func =
            async function ()
            {
                choose_ui_block("preparing");
                set_status_message("Preparing factory reset");
                return await backend.factory_reset.call(backend,uuid);
            };
        let done_func =
            function (r)
            {
                choose_ui_block("finished");
                set_status_message("Ready to reboot and reset");
            };
        let fail_func = 
            function ()
            {
                choose_ui_block("intro");
                set_status_message("Factory reset failed");
            };
        
        await backend.run_session(start_func, done_func, fail_func,
                                  set_status_message);
    }
    
    async check_update_status ()
    {
        let backend  = this.backend;
        let frontend = this;
        let start_func =
            async function ()
            { 
                choose_ui_block("checking");
                set_status_message("Checking OS Status");
                return await backend.os_status.call(backend);
            };
        let done_func  =
            function (r)
            {
                choose_ui_block("checking");
                set_status_message("OS status check complete");
                frontend.get_os_status.call(frontend, r.uuid);
            };
        let fail_func  =
            function ()
            {
                choose_ui_block("intro");
                set_status_message("OS status check failed");
            };
        
        await backend.run_session(start_func, done_func, fail_func,
                                  set_status_message);
    }

    async unprepare_factory_reset ()
    {
        let backend = this.backend;
        let start_func =
            async function ()
            {
                choose_ui_block("undoing");
                set_status_message("Deconfiguring factory reset");
                return await backend.undo_reset.call(backend);
            };
        let done_func  =
            function (r)
            {
                choose_ui_block("intro");
                set_status_message("Factory reset cancelled");
            };
        let fail_func  = 
            function ()
            {
                choose_ui_block("intro");
                set_status_message("Undo reset failed");
            };

        await backend.run_session(start_func, done_func, fail_func,
                                  set_status_message);        
    }
    
} //end class Reset

export function debug_log (line,msg)
{
    let dlog = document.getElementById('debug-log');
    if (dlog)
        dlog.textContent += line + ': ' +  msg + "\n";
}

let reset = new Reset();

export function check_status (el)
{
    reset.check_update_status();
}

export function prepare_reset (el)
{
    reset.prepare_factory_reset();
}

export function undo_reset (el)
{
    reset.unprepare_factory_reset();
}
