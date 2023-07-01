"use strict";

(function () {
    var AppUi = Script.require('appUi');

    function gotoOpened() {
        shouldShowDot = false;
        ui.messagesWaiting(shouldShowDot);
    }

    var shouldShowDot = false;

    var ui;
    var TEST_QML_SOURCE = "qrc:///qml/hifi/tablet/WebViewTest.qml";
    var BUTTON_NAME = "WEBVIEW";

    function startup() {
        ui = new AppUi({
            buttonName: BUTTON_NAME,
            normalButton: "icons/tablet-icons/goto-i.svg",
            activeButton: "icons/tablet-icons/goto-a.svg",
            sortOrder: 8,
            onOpened: gotoOpened,
            home: TEST_QML_SOURCE,
        });
    }

    startup();
}());
