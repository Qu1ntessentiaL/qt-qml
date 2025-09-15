import QtQuick
import QtQuick.Controls
import MyLib 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 480
    title: "COM-port demo"

    SerialManager {
        id: serial
    }

    Column {
        anchors.centerIn: parent
        spacing: 10

        ComboBox {
            id: portSelector
            model: serial.ports
            onCurrentTextChanged: serial.portName = currentText
        }

        Button {
            id: openButton
            text: serial.isOpen ? "Port Open" : "Open Port"
            background: Rectangle {
                color: serial.isOpen ? "green" : "red"
                radius: 6
            }
            onClicked: {
                if (serial.isOpen)
                    serial.closePort()
                else
                    serial.openPort()
            }
        }

        TextArea {
            id: logArea
            width: 350
            height: 150
            readOnly: true
            text: serial.receivedData
        }

        TextField {
            id: addressField
            width: 120
            placeholderText: "Slave address"

            validator: RegularExpressionValidator {
                regularExpression: /0x[0-9A-Fa-f]{0,2}/
            }

            inputMethodHints: Qt.ImhPreferUppercase | Qt.ImhHexadecimal

            onActiveFocusChanged: {
                if (activeFocus && text === "")
                    text = "0x"
            }

            onTextChanged: {
                if (!text.startsWith("0x")) {
                    text = "0x"
                    addressField.cursorPosition = text.length
                }
            }
        }

        Row {
            spacing: 5
            TextField {
                id: inputField
                width: 200
                placeholderText: "Send text"
            }
            Button {
                text: "Send"
                onClicked: serial.sendData(inputField.text)
            }
        }
    }

    Connections {
        target: serial
        onErrorOccurred: console.log("Serial Error: " + msg)
    }
}
