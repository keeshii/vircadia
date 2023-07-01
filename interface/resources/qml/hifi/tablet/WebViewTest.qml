//
//  Created by Dante Ruiz on 3/4/19.
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


import QtQuick 2.5

import QtWebView 1.1
import stylesUit 1.0
import controlsUit 1.0 as HifiControls


Rectangle {
    id: root
    anchors.fill: parent
    property string pluginName: ""
    property var displayInformation: null
    HifiConstants { id: hifi }

    WebView {
        id: webView
        anchors.fill: root
        url: "https://google.com"
    }

}
