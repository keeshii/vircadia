import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.12

import stylesUit 1.0 as HifiStylesUit
import controlsUit 1.0 as HifiControls
import "qrc:////qml//styles" as HifiStyles
import "qrc:////qml//hifi" as Hifi

Rectangle {
    id: emoteApp
    color: "#2b2b2b"

    HifiStylesUit.HifiConstants { id: hifiUi }
    HifiStyles.HifiConstants { id: hifi }

    Rectangle {
        id: headerBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 80
        color: hifiUi.colors.black
        z: 1

        Text {
            id: header
            color: "#ffffff"
            text: "Emote App"
            font.pixelSize: hifi.fonts.headerPixelSize
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            font.weight: Font.Bold
            anchors.fill: parent
            anchors.rightMargin: 2 * hifi.layout.spacing
            anchors.leftMargin: 2 * hifi.layout.spacing
        }
    }

    Rectangle {
        id: descriptionBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerBar.bottom
        height: 40
        color: "#2b2b2b"
        z: 1

        Text {
            id: description
            color: "#ffffff"
            text: "Choose an emote"
            anchors.fill: parent
            font.pixelSize: 16
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            anchors.rightMargin: 2 * hifi.layout.spacing
            anchors.leftMargin: 2 * hifi.layout.spacing
            anchors.topMargin: 2 * hifi.layout.spacing
        }
    }

    GridView {
      id: emoteList
      anchors.top: descriptionBar.bottom
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottomMargin: hifi.layout.spacing
      anchors.rightMargin: 0
      anchors.leftMargin: hifi.layout.spacing
      anchors.topMargin: hifi.layout.spacing
      cellHeight: 90 + hifi.layout.spacing
      cellWidth: (parent.width - hifi.layout.spacing) / 2
      z: 0

      ScrollBar.vertical: ScrollBar {
          id: listView
          anchors.right: parent.right
          visible: listView.contentHeight > listView.height ? true : false
      }

      delegate: HifiControls.Button {
          id: emoteButton;
          enabled: true;
          color: hifiUi.buttons.white
          text: name
          height: 90
          width: emoteList.width / 2 - hifi.layout.spacing
          onClicked: sendEmote(name)
      }

      model: ListModel {
          id: model
          ListElement { name: "Crying" }
          ListElement { name: "Surprised" }
          ListElement { name: "Dancing" }
          ListElement { name: "Cheering" }
          ListElement { name: "Waving" }
          ListElement { name: "Fall" }
          ListElement { name: "Pointing" }
          ListElement { name: "Clapping" }
          ListElement { name: "Sit" }
          ListElement { name: "Love" }
      }
    }

    Component.onDestruction: {
      stop();
    }

    function stop() {}

    function fromScript(message) {}

    function sendEmote(value) {
        var clickEvent = { type: "click", data: value };
        emoteApp.sendToScript(JSON.stringify(clickEvent));
    }

    signal sendToScript(var message);
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.75;height:480;width:640}D{i:1}D{i:2}D{i:3}D{i:4}
D{i:7}D{i:8}D{i:9}D{i:6}D{i:10}D{i:5}D{i:13}D{i:14}D{i:20}D{i:21}D{i:22}
}
##^##*/
