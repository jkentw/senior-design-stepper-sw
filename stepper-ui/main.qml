import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Window {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    title: qsTr("Stepper Control")

    /*
    GridLayout {
        id: mainLayout
        anchors.fill: parent

        columns: 4
        rows: 5

        FileSelectItem {
            id: recipeSelect
            prompt: "Choose Recipe"
            //processFileFunction: RecipeCpp.readRecipe(FileSelectCpp.filePath)

            Layout.row: 1
            Layout.column: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25

            Connections {
                target: FileSelectCpp

                function onFilePathChanged(str) {
                    RecipeCpp.readRecipe(str)
                }
            }
        }

        Label {
            Layout.row: 2
            Layout.rowSpan: 2
            Layout.column: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25

            text: "recipe placeholder text"
        }

        Button {
            id: startStopBtn
            text: "START"
            enabled: false

            Layout.row: 4
            Layout.column: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25

            onClicked: {
                //ProcessControlCpp.abc()
            }
        }

        Label {
            Layout.row: 5
            Layout.column: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25

            text: "progress placeholder text (M/N dies patterned)"
        }

        ProgressBar {
            id: recipeProgress
            value: 0.65

            Layout.row: 6
            Layout.column: 1
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25
        }

        Image {
            id: patternPreview
            Layout.row: 6
            Layout.column: 1
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25
        }

        Label {
            id: waferView
            text: "wafer view placeholder"

            Layout.row: 2
            Layout.rowSpan: 2
            Layout.column: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25
        }

        Label {
            id: processStatus
            text: "status placeholder (stage position, current state)"

            Layout.row: 4
            Layout.rowSpan: 2
            Layout.column: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25
        }


    }
    */

    Button {
        id: i2cTestBtn

        onClicked: {
            I2cTestCpp.invoke(undefined)
        }

        anchors {
            left: parent.left
            top: parent.top
            margins: 25
        }

        width: 160
        height: 40
        text: "Test I2C"
    }

    Button {
        id: projectorTestBtn

        onClicked: {
            if(Qt.application.screens[1] !== undefined) {
                if(!projectorWindow.visible) {
                    projectorWindow.setX(Qt.application.screens[0].width)
                    projectorWindow.showFullScreen()
                }

                ProjectorTestCpp.invoke(undefined)
            }
        }

        anchors {
            left: parent.left
            top: i2cTestBtn.bottom
            margins: 25
        }

        width: 160
        height: 40
        text: "Test Projector"
    }

    Window {
        id: projectorWindow
        width: 320
        height: 240

        Image {
            id: projectorImg
            cache: false

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                top: parent.top

                margins: 25
            }
        }

        Connections {
            target: ProjectorCpp

            function onUpdateImage(img) {
                projectorImg.source = ""
                projectorImg.source = "image://projector/image"
            }
        }
    }

    //camera test
    Button {
        id: captureBtn

        onClicked: {
            CameraTestCpp.invoke(undefined)
        }

        anchors {
            left: parent.left
            top: projectorTestBtn.bottom
            margins: 25
        }

        width: 160
        height: 40
        text: "Capture Image"
    }

    Image {
        id: cameraFeed
        cache: false

        anchors {
            left: captureBtn.right
            top: parent.top

            margins: 25
        }

        sourceSize.width: 160
        sourceSize.height: 120
    }

    Connections {
        target: CameraLiveCpp

        function onUpdateImage(img) {
            cameraFeed.source = ""
            cameraFeed.source = "image://camera_live/image"
        }
    }

    Image {
        id: cameraCapture
        cache: false

        anchors {
            left: cameraFeed.right
            top: parent.top

            margins: 25
        }

        sourceSize.width: 320
        sourceSize.height: 240
    }

    Connections {
        target: CameraStillCpp

        function onUpdateImage(img) {
            cameraCapture.source = ""
            cameraCapture.source = "image://camera_still/image"
        }
    }

    //file upload class test
    FileSelectItem {
        prompt: "Choose Recipe"

        anchors {
            margins: 25
            top: captureBtn.bottom
            left: parent.left
        }

        Connections {
            target: FileSelectCpp

            function onFilePathChanged(str) {
                RecipeTestCpp.invoke(str)
            }
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
