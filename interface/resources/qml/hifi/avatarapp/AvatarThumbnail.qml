import QtQuick 2.9
import QtGraphicalEffects 1.0

Item {
    width: 92
    height: 92
    property alias wearableIndicator: indicator

    property int wearablesCount: 0
    onWearablesCountChanged: {
        console.debug('AvatarThumbnail: wearablesCount = ', wearablesCount)
    }

    property url externalAvatarThumbnailUrl;
    property var avatarUrl;

    Image {
        id: avatarImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop

        Binding on source {
            when: avatarUrl !== ''
            value: avatarUrl
        }

        visible: avatarImage.status !== Image.Loading && avatarImage.status !== Image.Error
    }

    Image {
        id: customAvatarImage
        anchors.fill: avatarImage;
        visible: avatarUrl === '' || avatarImage.status === Image.Error
        source: externalAvatarThumbnailUrl
    }

    Rectangle {
        anchors.fill: parent;
        color: 'white'
        visible: avatarImage.status === Image.Loading
        radius: avatarImage.radius
        border.width: avatarImage.border.width
        border.color: avatarImage.border.color

        Spinner {
            id: spinner
            visible: parent.visible
            anchors.fill: parent;
        }
    }

    AvatarWearablesIndicator {
        id: indicator
        anchors.left: avatarImage.left
        anchors.bottom: avatarImage.bottom
        anchors.leftMargin: 57
        wearablesCount: parent.wearablesCount
        visible: parent.wearablesCount !== 0
    }
}
