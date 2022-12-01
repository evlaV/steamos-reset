// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Window 2.12
import QtQuick.Controls.Material 2.12

import "qrc:/js/reset.js" as Core

ApplicationWindow
{
    id: root
    title: qsTr("SteamOS Factory Reset")
    width: 800
    height: 480
    visible: true

    function fg() { return Material.foreground; }
    function bg() { return Material.background; }

    // Expose QML i18n/l10n to the JS layer:
    function translate (txt) { return qsTr(txt) }

    // put QML elements where the javascript application logic can see them:
    // (only items we expect will be accepted, see the this.glue slot in js)
    function register_ui  (element,name) { Core.reset.attach(name,element,"ui"); }
    function register_func(func,name)    { Core.reset.attach(name,func,"func");  }

    // ==================================================================
    // QML javascript doesn't have timeout/promises/async/etc
    // but it _can_ use Timer objects we have created for it here:
    Timer { id: timer }

    function setup_timer(ms,callback) 
    {
        timer.interval = ms;
        timer.repeat = false;
        timer.triggered.connect(callback);
    }
    function clear_timer(callback) { timer.triggered.disconnect(callback); }
    function restart_timer() { timer.restart(); }
    function stop_timer()    { timer.stop();    }
    
    Component.onCompleted:
    {
        register_func(clear_timer,   'timer_clear');
        register_func(setup_timer,   'timer_init' );
        register_func(restart_timer, 'timer_reset');
        register_func(stop_timer,    'timer_stop' );
        register_func(translate,     'translate'  );
    }
    // ==================================================================

    Column
    {
        id: ui
        height: parent.height
        width: parent.width

        Rectangle
        {
            id: header
            border.width: 2
            radius: 8
            visible: true
            width: parent.width
            color: bg()
            border.color: fg()
            height: title.height + 10
            
            Label
            {
                id: title
                text: "SteamOS Factory Reset"
                anchors.centerIn: parent
                font.pointSize: 24
            }
        }

        Row
        {
            Rectangle
            {
                id: info
                width: ui.width / 2
                height: ui.height - log.height - header.height - footer.height
                border.width: 2
                radius: 8
                color: bg()
                border.color: fg()

                Column
                {
                    id: intro
                    x: parent.radius
                    y: parent.radius
                    spacing: 8
                    visible: true

                    Label { text: qsTr(" This tool will:") }
                    Label { text: qsTr("  • Check That your OS image is unmodified") }
                    Label { text: qsTr("  • Download and install a fresh image if necessary") }
                    Label { text: qsTr("  • Set up your Deck to erase games & user data when it reboots") }
                }

                Column
                {
                    id: checking
                    x: parent.radius
                    y: parent.radius
                    visible: false

                    Component.onCompleted: { register_ui(checking,'checking') }

                    Label { text: qsTr("Checking OS Status") }
                }

                Column
                {
                    id: prepare
                    x: parent.radius
                    y: parent.radius
                    visible: false
                    spacing: 8

                    Component.onCompleted: { register_ui(prepare,'prepare') }
                
                    Label { text: qsTr(" Next:") }
                    Label
                    {
                        id: freshen
                        visible: false
                        text: qsTr("  • Install a fresh SteamOS image")
                        Component.onCompleted: { register_ui(freshen,'need-update') }
                    }
                    Label { text: qsTr("  • Prepare to erase user data on reboot") }
                }

                Column
                {
                    id: preparing
                    x: parent.radius
                    y: parent.radius
                    visible: false

                    Component.onCompleted: { register_ui(preparing,'preparing') }

                    Label { text: qsTr("Preparing for factory reset") }
                }

                Column
                {
                    id: finished
                    x: parent.radius
                    y: parent.radius
                    visible: false

                    Component.onCompleted: { register_ui(finished,'finished') }

                    Label { text: qsTr("Factory Reset: Ready") }
                    Label { text: qsTr("Reboot your deck to proceed") }
                }

                Column
                {
                    id: undoing
                    x: parent.radius
                    y: parent.radius
                    visible: false

                    Component.onCompleted: { register_ui(undoing,'undoing') }

                    Label { text: qsTr("Removing reset configuration") }
                }
            }

            Rectangle
            {
                id: status
                width: ui.width / 2
                height: info.height
                border.width: 2
                radius: 8
                color: bg()
                border.color: fg()

                Column
                {
                    spacing: 8
                    x: status.radius
                    y: status.radius

                    Label { text: qsTr("Status:"); }
                    Row { Label { text: qsTr("Running Image: ") }
                          Label { id: this_boot; text: "-" } }
                    Row { Label { text: qsTr("Next Boot Image: ") }
                          Label { id: next_boot; text: "-" } }
                    Label { text: qsTr("Actions on next reboot:") }
                    Label
                    {
                        x: parent.x * 2
                        id: reset_list;
                        text: "none"
                    }
                }
            }

            Component.onCompleted:
            {
                register_ui(this_boot ,'this_boot' );
                register_ui(next_boot ,'next_boot' );
                register_ui(reset_list,'reset_list');
                register_ui(intro,'intro');
                Core.reset.set_stage('intro');
            }
        }

        Rectangle
        {
            id: log
            border.width: 2
            radius: 8
            visible: true
            width: parent.width
            height: (parent.height - header.height - footer.height) * 0.5
            color: bg()
            border.color: fg()

            Label
            {
                id: echo
                text: ""
                height: parent.height - 8
                width: parent.width
                x: parent.radius
                y: parent.radius
                font.family: "Monospace"
                wrapMode: Text.Wrap
            }

            Component.onCompleted: { register_ui(echo,'log') }
        }        

        Rectangle
        {
            id: footer
            border.width: 2
            radius: 8
            visible: true
            width: parent.width
            height: header.height
            color: bg()
            border.color: fg()

            RoundButton
            {
                id: activate
                anchors.centerIn: parent
                text: "Check Status"
                height: parent.height - 8
                icon.name: "media-playback-start"
                display: AbstractButton.TextBesideIcon
                radius: 3
                onClicked: { Core.reset.clicked(activate) }

                RotationAnimator on rotation
                {
                    id: spinner
                    from: 0
                    to: 360
                    duration: 1000
                    running: false
                    loops: Animation.Infinite
                }

                Component.onCompleted:
                {
                    register_ui(activate,'activate');
                    register_ui(spinner, 'spinner' );
                }
            }
        }
    }
}
