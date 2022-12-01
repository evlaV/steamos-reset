// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

.import "./backend.js" as Service

function max (a,b) { return (a >= b) ? a : b }
function min (a,b) { return (a <= b) ? a : b }

class Reset
{
    constructor ()
    {
        this.backend = new Service.Backend("http://127.0.0.1:8080/cgi");
        this.reset_id = 0;
        this.update_needed = false;
        this.update_url = "";
        this.stage = 'intro';
        this.service = { "uuid": false,
                         "result": null,
                         "count": 0,
                         "msgidx": 0 },
        this.icons =
            {
                "clickable":"qrc:/icons/activate.png",
                "clicked":"qrc:/icons/spinner.png",
            };
        this.cache = {"dummy":false};
        this.glue =
            {
                "intro"  :  { "label" : "Check Status",
                              "icon"  : "next",
                              "next"  : "checking",
                              "ui"    : undefined,
                              "init"  : this.show_boot_status,
                              "button": this.check_update_status },
                "checking": { "label" : false,
                              "next"  : "prepare",
                              "ui"    : undefined,
                              "button": false },
                "prepare" : { "label" : "Prepare for Reset",
                              "icon"  : "next",
                              "next"  : "preparing",
                              "ui"    : undefined,
                              "button": this.prepare_reset },
                "preparing":{ "label" : false,
                              "next"  : "finished",
                              "ui"    : undefined,
                              "button": false },
                "finished": { "label" : "What? No! Undo!",
                              "icon"  : "start",
                              "next"  : "undoing",
                              "init"  : this.show_boot_status,
                              "ui"    : undefined,
                              "button": this.unprepare_reset },
                "undoing":  { "label" : false,
                              "next"  : "intro",
                              "ui"    : undefined,
                              "button": false },
                "log"        : { "ui": undefined },
                "activate"   : { "ui": undefined },
                "spinner"    : { "ui": undefined },
                "need-update": { "ui": undefined },
                "this_boot"  : { "ui": undefined },
                "next_boot"  : { "ui": undefined },
                "reset_list" : { "ui": undefined },
                "timer_init"   : { "func": undefined },
                "timer_clear"  : { "func": undefined },
                "timer_reset"  : { "func": undefined },
                "timer_stop"   : { "func": undefined },
                "translate"    : { "func": undefined },
            };
    }

    xlate (txt)
    {
        if (this.glue.translate.func)
            return this.glue.translate.func(txt);
        return txt;
    }

    init_timer (ms,callback)
    {
        if (this.glue.timer_init.func)
            this.glue.timer_init.func(ms, callback);
    }

    clear_timer (callback)
    {
        if (this.glue.timer_clear.func)
            this.glue.timer_clear.func(callback);
    }
    
    restart_timer ()
    {
        if (this.glue.timer_reset.func)
            this.glue.timer_reset.func();
    }

    stop_timer ()
    {
        if (this.glue.timer_stop.func)
            this.glue.timer_stop.func();
    }
    
    attach (stage,object,type)
    {
        if( !type )
            type = "ui";

        try { this.glue[stage][type] = object; }
        catch (err)
        {
            console.log("QML/JS attach error: "   +
                        stage + "." + type + ": " + err.message );
        }
    }

    log_message (msg)
    {
        if (this.glue.log.ui)
            this.glue.log.ui.text = msg;
    }

    init_service_data ()
    {
        this.service.uuid = null;
        this.service.result = null;
        this.service.msgidx = 0;
        this.service.count = 0;
    }
    
    run_service (service,ondone,onfail,args)
    {
        console.log("RUN " + service);
        this.init_service_data();

        let self = this;
        let res =
            args ? service.apply(this.backend,args) : service.call(this.backend);
        let result = res.result;
        let uuid   = result.uuid;


        if (!ondone)
            ondone = function (r) { console.log("$generic.ondone");
                                    self.advance_stage.call(self); };

        if (!onfail)
            onfail = function () { self.set_stage.call(self, "intro"); };

        switch (res.state)
        {
            case "FINISHED":
              console.log("FINISHED " + uuid);
              ondone(result);
              break;

            case "PENDING":
              console.log("PENDING " + uuid);
              let echo = function (x) { self.log_message.call(self,x); };
              let poll =
                function ()
                {
                    let status =
                        self.backend.poll_session.call(self.backend,
                                                       uuid,
                                                       self.service.msgidx,
                                                       echo);
                    switch (status)
                    {
                        case -2:
                          onfail();
                          self.stop_timer.call(self);
                          self.clear_timer.call(self,poll);
                          break;

                        case -1:
                          console.log("ondone " + uuid);
                          self.stop_timer.call(self);
                          self.clear_timer.call(self,poll);
                          ondone(result);
                          break;

                        default:
                          if( status > 0 )
                              self.service.msgidx += status;
                          self.service.count++;
                          self.restart_timer.call(self);
                    }
                };
              this.init_timer(1000, poll);
              this.restart_timer();
              break;

            case "FAILED":
            default:
              let errstr = res ? res.message : "Service Unavailable";
              this.log_message( errstr );
              onfail();
              break;
        }
    }

    prepare_reset ()
    {
        let args = [this.service.uuid];
        this.run_service(this.backend.factory_reset, false, false, args);
    }

    unprepare_reset ()
    {
        this.run_service(this.backend.undo_reset);
    }

    show_boot_status ()
    {
        let res = this.backend.boot_status();
        let boot_state = (res && res.result && res.result.boot) ?
            res.result.boot : {};
        let to_reset   = boot_state["reset-list"] || [];

        let this_boot = this.glue['this_boot'].ui;
        let next_boot = this.glue['next_boot'].ui;
        let rlist     = this.glue['reset_list'].ui;

        console.log("called boot_status " + res.state);
        
        if( res.state == "FINISHED" )
        {
            if (this_boot)
                this_boot.text = boot_state.current || "?";

            if (next_boot)
                next_boot.text = boot_state.next    || "?";

            if (rlist && to_reset)
            {
                let text = "";
                let erase = this.xlate("Erase ");
                for (var item of to_reset)
                {
                    if (!item)
                        continue;

                    let line = erase + item.label + " (" + item.device + ")\n";
                    text += line;
                }

                rlist.text = text || "<i>" + this.xlate("none") + "</i>";
            }
        }
        else
        {
            if (this_boot)
                this_boot.text = "?";

            if (next_boot)
                this_boot.text = "?";
        }

        
    }
    
    check_update_status ()
    {
        let self = this;
        let ondone =
            function (r)
            {
                let res = self.backend.os_status(r.uuid);

                if (res.state != "FINISHED")
                {
                    console.log("ERROR: Status data missing: " + res.message );
                    self.set_stage('intro');
                    return;
                }

                self.service.uuid = r.uuid;
                
                if (res && res.result && res.result.update )
                {
                    self.update_needed =
                        res.result.update.needed * 1 ? true : false;
                    self.update_url    = res.result.update.url || "";
                }
                else
                {
                    self.update_needed = false;
                    self.update_url = "";
                }

                console.log("UPDATE: " + self.update_needed);
                console.log(self.glue["need-update"].ui);
                
                if (self.glue["need-update"].ui)
                    self.glue["need-update"].ui.visible = self.update_needed;

                console.log("check_update_status.ondone");
                self.advance_stage.call(self);
            };

        this.run_service(this.backend.os_status, ondone, false);
    }
    
    set_stage (name,button)
    {
        let prev  = this.stage;
        let stage = this.glue[name];

        console.log("set-stage: " + name + " -> " + stage);

        if (!button)
        {
            button = this.glue.activate.ui;
        }

        if (!stage)
            return false;

        console.log("set-stage: " + name + " -> " + stage.ui);

        if (stage.ui)
        {
            let before = this.glue[prev];

            if (before && before.ui)
            {
                before.ui.visible = false;
            }

            console.log("set-stage: " +
                        "- " + prev + "; " +
                        "+ " + name + "; " +
                        "init: " + stage.init );
            
            if( stage.init && stage.init.call )
                stage.init.call(this);
            
            stage.ui.visible = true;
            this.stage = name;
        }

        if (!button)
            return stage.ui ? true : false;

        let spinner = this.glue.spinner.ui;

        if (stage.label)
        {
            spinner.stop();

            button.rotation = 0;
            button.radius   = this.cache["spinner.radius"] || 3;
            button.width    = undefined;
            button.height   = button.parent.height - 16;

            button.text        = stage.label;
            button.icon.source = this.icons.clickable;
        }
        else
        {
            let diameter = min(button.parent.width,button.parent.height) - 16;

            button.text        = "";
            button.icon.source = this.icons.clicked;

            this.cache["spinner.radius"] = button.radius;
            this.cache["button.width"  ] = button.width;
            this.cache["button.height" ] = button.height;
            button.radius = diameter / 2;
            button.width  = diameter;
            button.height = diameter;

            spinner.start();
        }

        button.icon.name = stage.icon;
        button.enabled   = stage.button ? true : false;

        return stage.ui ? true : false;
    }

    advance_stage (button)
    {
        let action = this.glue[this.stage];

        console.log("ADVANCE " + this.stage + "->" + action.next);

        if (action)
        {
            console.log("call set-stage(" + action.next + ")");
            this.set_stage(action.next, button);
        }

        if (action.button)
            action.button.call(this);
    }
    
    clicked (button)
    {
        console.log("CLICK");
        this.advance_stage(button);
    }
}

let reset = new Reset();
