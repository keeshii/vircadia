import Hifi 1.0 as Hifi
import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.12

import stylesUit 1.0 as HifiStylesUit
import controlsUit 1.0 as HiFiControls
import controls 1.0 as HifiControls
import "qrc:////qml//hifi" as Hifi
import "qrc:////qml//styles"

Rectangle {
  id: rectLayout
  color: "#433952"

  property bool keyboardEnabled: false
  property bool punctuationMode: false
  property bool keyboardRaised: false

  function setStep(stepNum) {
    stepList.completed = stepNum
    switch (stepNum) {
      case 0:
        loader.sourceComponent = step1;
        break;
      case 1:
        loader.sourceComponent = step2;
        break;
      case 2:
        loader.sourceComponent = step3;
        break;
      case 3:
        loader.sourceComponent = step4;
        break;
      case 4:
        loader.sourceComponent = step5;
        break;
      default:
        loader.setSource(undefined);
    }
  }

  function completeWizard() {
    var completionMessage = {
      command: "complete-wizard",
      data: {
        performancePreset: loader.performancePreset,
        refreshRateProfile: loader.refreshRateProfile,
        displayName: loader.displayName
      }
    };
    eventBridge.emitWebEvent(JSON.stringify(completionMessage));
  }

  function handleWebEvent(message) {
    var messageJSON = JSON.parse(message);
    if (messageJSON.command === "script-to-web-initialize") {
      loader.performancePreset = messageJSON.data.performancePreset;
      loader.refreshRateProfile = messageJSON.data.refreshRateProfile;
      loader.displayName = messageJSON.data.displayName;
    }
  }

  function initializeWizard() {
    var initializeCommand = {"command": "first-run-wizard-ready"};
    eventBridge.emitWebEvent(JSON.stringify(initializeCommand));
  }

  // Layout constants constants
  HifiConstants { id: hifi }

  Rectangle {
    id: steps
    color: "#26202e"
    width: parent.width - 8 * hifi.layout.spacing
    height: hifi.layout.rowHeight + 6 * hifi.layout.spacing
    anchors.top: rectLayout.top
    anchors.topMargin: 4 * hifi.layout.spacing
    anchors.horizontalCenter: rectLayout.horizontalCenter
    radius: hifi.layout.spacing

    ListView {
      id: stepList
      anchors.fill: parent
      orientation: ListView.Horizontal

      property int completed: 0

      delegate: Item {
        id: stepItem
        width: stepList.width / 5
        height: stepList.height
        property int num: index

        Rectangle {
            width: parent.width
            height: 1
            anchors.left: stepCircle.horizontalCenter
            anchors.top: stepCircle.verticalCenter
            visible: stepItem.num + 1 < stepList.model.count
            color: stepList.completed > stepItem.num ? "#4bb543" : "gray"
        }

        Rectangle {
          id: stepCircle
          color: stepList.completed > stepItem.num ? "#4bb543"
               : (stepList.completed === stepItem.num ? "white" : "gray")
          width: hifi.layout.rowHeight
          height: hifi.layout.rowHeight
          radius: hifi.layout.rowHeight / 2
          anchors.top: stepItem.top
          anchors.topMargin: hifi.layout.spacing
          anchors.horizontalCenter: stepItem.horizontalCenter

          Text {
            id: stepNum
            text: String(stepItem.num + 1)
            color: stepList.completed === stepItem.num ? hifi.colors.text : "white"
            anchors.centerIn: stepCircle
          }
        }

        Text {
          id: stepName
          text: name
          color: "white"
          anchors.top: stepCircle.bottom
          anchors.topMargin: hifi.layout.spacing
          anchors.horizontalCenter: stepItem.horizontalCenter
          font.bold: true
        }
      }

      model: ListModel {
        ListElement { name: "Welcome" }
        ListElement { name: "Quality" }
        ListElement { name: "Performance" }
        ListElement { name: "Display Name" }
        ListElement { name: "Completion" }
      }

    }
  }

  Loader {
    id: loader
    width: parent.width - 8 * hifi.layout.spacing
    height: parent.height - steps.height - backButton.height - 12 * hifi.layout.spacing
    anchors.top: steps.bottom
    anchors.topMargin: 2 * hifi.layout.spacing
    anchors.horizontalCenter: rectLayout.horizontalCenter
    sourceComponent: step1

    property int performancePreset: 0
    property int refreshRateProfile: 0
    property string displayName: ""
  }

  Component {
    id: step1

    Item {
      id: step1Body
      anchors.fill: loader

      Text {
        id: step1Header
        width: parent.width
        text: "Welcome to Overte!"
        color: "white"
        font.pixelSize: hifi.fonts.headerPixelSize
        font.bold: true

        anchors.top: step1Body.top
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Text {
        id: step1Text1
        width: parent.width
        text:
          "Let's get you setup to experience the virtual world.<br />" +
          "First, we need to select some performance and graphics quality options.<br />" +
          "<br />" +
          "Press <b><font color=\"#2e89ff\">Continue</font></b> when you are ready."
        color: "white"
        wrapMode: Text.Wrap
        textFormat: TextEdit.RichText

        anchors.top: step1Header.bottom
        anchors.topMargin: 2 * hifi.layout.spacing
      }
    }
  }

  Component {
    id: step2

    Item {
      id: step2Body
      anchors.fill: loader

      Text {
        id: step2Header
        width: parent.width
        text: "Quality"
        color: "white"
        font.pixelSize: hifi.fonts.headerPixelSize
        font.bold: true

        anchors.top: step2Body.top
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Text {
        id: step2Text1
        width: parent.width
        text:
          "What level of visual quality would you like?<br />" +
          "<b>Remember! If you do not have a powerful computer,<br />" +
          "you may want to set this to low or medium at most.</b>"
        color: "white"
        wrapMode: Text.Wrap
        textFormat: TextEdit.RichText

        anchors.top: step2Header.bottom
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      ColumnLayout {
        anchors.top: step2Text1.bottom
        anchors.topMargin: 2 * hifi.layout.spacing

        RadioButton {
            text: 
              "<font size=\"4\" color=\"#ff9900\"><b>Very Low Quality</b></font>\n" +
              "<font color=\"white\">Slow Laptop / Very Slow Computer</font>"
            onClicked: loader.performancePreset = 1
            checked: loader.performancePreset === 1
        }
        RadioButton {
            text:
              "<font size=\"4\" color=\"#ffff00\"><b>Low Quality</b></font>\n" +
              "<font color=\"white\">Average Laptop / Slow Computer</font>"
            onClicked: loader.performancePreset = 2
            checked: loader.performancePreset === 2
        }
        RadioButton {
            text:
              "<font size=\"4\" color=\"#00ba1c\"><b>Medium Quality</b></font>\n" +
              "<font color=\"white\">Average Computer - </font><font color=\"#00ba1c\"><i>Recommended</i></font>"
            onClicked: loader.performancePreset = 3
            checked: loader.performancePreset === 3
        }
        RadioButton {
            text:
              "<font size=\"4\" color=\"#0096db\"><b>High Quality</b></font>\n" +
              "<font color=\"white\">Gaming Computer</font>"
            onClicked: loader.performancePreset = 4
            checked: loader.performancePreset === 4
        }
      }
    }
  }

  Component {
    id: step3

    Item {
      id: step3Body
      anchors.fill: loader

      Text {
        id: step3Header
        width: parent.width
        text: "Performance"
        color: "white"
        font.pixelSize: hifi.fonts.headerPixelSize
        font.bold: true

        anchors.top: step3Body.top
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Text {
        id: step3Text1
        width: parent.width
        text:
          "Do you want a smooth experience <i>(high refresh rate)</i><br />" +
          "or do you want to conserve power and resources <i>(low refresh rate)</i> on your computer?<br />" +
          "<b><i>Note: This does not apply to virtual reality headsets.</i><b>"
        color: "white"
        wrapMode: Text.Wrap
        textFormat: TextEdit.RichText

        anchors.top: step3Header.bottom
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      ColumnLayout {
        anchors.top: step3Text1.bottom
        anchors.topMargin: 2 * hifi.layout.spacing

        RadioButton {
            text:
              "<font size=\"4\" color=\"#ff9900\"><b>Not Smooth (20 Hz)</b></font>\n" +
              "<font color=\"white\">Conserve Power</font>"
            onClicked: loader.refreshRateProfile = 1
            checked: loader.refreshRateProfile === 1
        }
        RadioButton {
            text:
              "<font size=\"4\" color=\"#ffff00\"><b>Smooth (30 Hz)</b></font>\n" +
              "<font color=\"white\">Use Average Resources</font>"
            onClicked: loader.refreshRateProfile = 2
            checked: loader.refreshRateProfile === 2
        }
        RadioButton {
            text:
              "<font size=\"4\" color=\"#00ba1c\"><b>Very Smooth (60 Hz)</b></font>\n" +
              "<font color=\"white\">Use Maximum Resources - </font><font color=\"#00ba1c\"><i>Recommended</i></font>"
            onClicked: loader.refreshRateProfile = 3
            checked: loader.refreshRateProfile === 3
        }
      }
    }
  }

  Component {
    id: step4

    Item {
      id: step4Body
      anchors.fill: loader

      Text {
        id: step4Header
        width: parent.width
        text: "Display Name"
        color: "white"
        font.pixelSize: hifi.fonts.headerPixelSize
        font.bold: true

        anchors.top: step4Body.top
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Text {
        id: step4Text1
        width: parent.width
        text:
          "What should people call you?<br />" +
          "This is simply a nickname, it will be shown in place of your username (if you have one)."
        color: "white"
        wrapMode: Text.Wrap
        textFormat: TextEdit.RichText

        anchors.top: step4Header.bottom
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Rectangle {
          id: addressBar
          width: parent.width
          height: 40
          color: 'white'
          anchors.top: step4Text1.bottom
          anchors.topMargin: 2 * hifi.layout.spacing

          TextField {
              id: displayName
              text: loader.displayName
              focus: true
              width: addressBar.width - addressBar.anchors.leftMargin - addressBar.anchors.rightMargin;
              anchors {
                  left: addressBar.left;
                  leftMargin: 8;
                  verticalCenter: addressBar.verticalCenter;
              }

              onTextChanged: loader.displayName = text
              placeholderText: "Enter display name"
              verticalAlignment: TextInput.AlignBottom
              onAccepted: {
                if (HMD.active) {
                  addressBarDialog.keyboardEnabled = false;
                }
              }

              font {
                  family: hifi.fonts.fontFamily
                  pixelSize: hifi.fonts.pixelSize * 0.75
              }

              color: hifi.colors.text
              background: Item {}
          }

          MouseArea {
              anchors.fill: parent;
              onClicked: {
                  displayName.focus = true;
                  displayName.forceActiveFocus();
                  if (HMD.active) {
                      addressBarDialog.keyboardEnabled = true;
                  }
              }
          }
      }
    }
  }

  Component {
    id: step5

    Item {
      id: step5Body
      anchors.fill: loader

      Text {
        id: step5Header
        width: parent.width
        text: "All done!"
        color: "white"
        font.pixelSize: hifi.fonts.headerPixelSize
        font.bold: true

        anchors.top: step5Body.top
        anchors.topMargin: 2 * hifi.layout.spacing
      }

      Text {
        id: step5Text1
        width: parent.width
        text:
          "Now you're almost ready to go!<br />" +
          "Press <font color=\"#1ee62e\">Complete</font> to save your setup.<br />" +
          "Then take a look at the other information kiosks after completing this wizard."
        color: "white"
        wrapMode: Text.Wrap
        textFormat: TextEdit.RichText

        anchors.top: step5Header.bottom
        anchors.topMargin: 2 * hifi.layout.spacing
      }
    }
  }

  Button {
    id: backButton
    text: "< Back"
    width: nextButton.width
    anchors.bottom: rectLayout.bottom
    anchors.left: rectLayout.left
    anchors.bottomMargin: 4 * hifi.layout.spacing
    anchors.leftMargin: 4 * hifi.layout.spacing
    visible: stepList.completed > 0
    onClicked: setStep(stepList.completed - 1)

    contentItem: Text {
        text: backButton.text
        font.bold: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        gradient: Gradient {
          GradientStop { position: 0 ; color: backButton.hovered ? "#0599fc" : "#0599fc" }
          GradientStop { position: 1 ; color: backButton.hovered ? "#003670" : "#002259" }
        }
        border.color: "#26282a"
        border.width: 1
        radius: 4
    }
  }

  Button {
    id: nextButton
    text: "Continue >"
    anchors.bottom: rectLayout.bottom
    anchors.right: rectLayout.right
    anchors.bottomMargin: 4 * hifi.layout.spacing
    anchors.rightMargin: 4 * hifi.layout.spacing
    visible: stepList.completed < 4
    onClicked: setStep(stepList.completed + 1)
    rightPadding: 2 * hifi.layout.spacing
    leftPadding: 2 * hifi.layout.spacing

    contentItem: Text {
        text: nextButton.text
        font.bold: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        gradient: Gradient {
          GradientStop { position: 0 ; color: nextButton.hovered ? "#0599fc" : "#0599fc" }
          GradientStop { position: 1 ; color: nextButton.hovered ? "#003670" : "#002259" }
        }
        border.color: "#26282a"
        border.width: 1
        radius: 4
    }
  }

  Button {
    id: completeButton
    text: "Complete"
    width: nextButton.width
    anchors.bottom: rectLayout.bottom
    anchors.right: rectLayout.right
    anchors.bottomMargin: 4 * hifi.layout.spacing
    anchors.rightMargin: 4 * hifi.layout.spacing
    visible: stepList.completed === 4
    onClicked: completeWizard()

    contentItem: Text {
        text: completeButton.text
        font.bold: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        gradient: Gradient {
          GradientStop { position: 0 ; color: nextButton.hovered ? "#59ffc2" : "#00ff00" }
          GradientStop { position: 1 ; color: nextButton.hovered ? "#196144" : "#003600" }
        }
        border.color: "#26282a"
        border.width: 1
        radius: 4
    }
  }

  HiFiControls.Keyboard {
      id: keyboard
      raised: parent.keyboardEnabled && parent.keyboardRaised
      numeric: parent.punctuationMode
      anchors {
          bottom: parent.bottom
          left: parent.left
          right: parent.right
      }
  }

  // Wait for the client-entity script to load before sending events
  Timer {
      id: timer
      function setTimeout(cb, delayTime) {
          timer.interval = delayTime;
          timer.repeat = false;
          timer.triggered.connect(cb);
          timer.triggered.connect(function release () {
              timer.triggered.disconnect(cb); // This is important
              timer.triggered.disconnect(release); // This is important as well
          });
          timer.start();
      }
  }

  Component.onCompleted: {
    eventBridge.scriptEventReceived.connect(handleWebEvent);
    timer.setTimeout(function(){ initializeWizard(); }, 2000);
  }

}
