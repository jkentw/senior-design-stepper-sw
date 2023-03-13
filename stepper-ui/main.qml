import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Stepper Control")

    //file upload class test
    FileSelectItem {
        prompt: "Choose Recipe"
        anchors {
            margins: 20
            top: parent.top
            left: parent.left
        }
    }

    /*
    //cross correlation test
    width: 640
    height: 480
    visible: true
    title: qsTr("Stepper Control")

    Button {
        id: patternFileSelectBtn
        onClicked: {
            ImageInputCpp.patternUpload()
        }

        anchors {
            left: parent.left
            top: parent.top
            margins: 25
        }

        width: 160
        height: 40
        text: "Choose Pattern"
    }

    Button {
        id: markFileSelectBtn
        onClicked: {
            ImageInputCpp.markUpload()
        }

        anchors {
            left: parent.left
            top: patternFileSelectBtn.bottom
            margins: 25
        }

        width: patternFileSelectBtn.width
        height: patternFileSelectBtn.height
        text: "Choose Alignment Mark"
    }

    Button {
        id: crossCorrBtn
        onClicked: {
            ImageInputCpp.crossCorrelation()
        }

        anchors {
            left: parent.left
            top: markFileSelectBtn.bottom
            margins: 25
        }

        width: patternFileSelectBtn.width
        height: patternFileSelectBtn.height
        text: "Locate Alignment Marks"

        enabled: false
    }

    Image {
        id: patternPreviewImg
        cache: false

        anchors {
            left: patternFileSelectBtn.right
            right: parent.right
            top: parent.top

            margins: 25
        }

        height: parent.height * 3 / 4 - 25

        sourceSize.width: width
        sourceSize.height: height
    }

    Image {
        id: markPreviewImg
        cache: false

        anchors {
            left: patternPreviewImg.left
            right: parent.right
            bottom: parent.bottom

            margins: 25
        }

        height: parent.height / 4 - 25

        sourceSize.width: width
        sourceSize.height: height
    }

    Connections {
        target: ImageInputCpp

        function onUpdatePattern(img) {
            //console.log(img)

            patternPreviewImg.source = "image://preview/"
            patternPreviewImg.source = "image://preview/pattern"
        }

        function onUpdateMark(img) {
            //console.log(img)

            markPreviewImg.source = "image://preview/"
            markPreviewImg.source = "image://preview/mark"
        }

        function onSetCrossCorrBtnEnabled(en) {
            crossCorrBtn.enabled = en
        }
    }
    */
}
