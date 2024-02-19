//
//  Created by Dante Ruiz on 3/4/19.
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


import QtQuick 2.5
import QtWebView 1.1
import QtWebSockets 1.1

Rectangle {
    id: root
    anchors.fill: parent

    WebView {
        id: webView
        anchors.fill: root
        url: "https://metaverse.kuro.ryuu.eu"
    }

    WebSocketServer {
        id: webSocketServer
        listen: false
    }
}
