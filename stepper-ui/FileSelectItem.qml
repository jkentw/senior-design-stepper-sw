import QtQuick 2.15
import QtQuick.Controls 2.0

Item {
    property string prompt: qsTr("Select File")

    implicitWidth: 240
    implicitHeight: 40

    Button {
        id: selectBtn
        text: parent.prompt
        width: 160
        height: 40

        onClicked: {
            FileSelectCpp.fileSelect()
        }
    }

    Text {
        id: pathText
        text: "No file selected."
        color: "gray"

        y: (parent.height - height)/2

        anchors {
            left: selectBtn.right
            leftMargin: 10
        }
    }

    Connections {
        target: FileSelectCpp

        function onFilePathChanged(str) {
            pathText.color = "blue"
            pathText.text = str
            console.log("path changed")
        }
    }
}
