import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    anchors.fill: parent

    property var discovery
    property var model

    // -------------------------
    // State machine
    // -------------------------
    states: [
        State { name: "idle";        when: discovery.state === 0 },
        State { name: "discovering"; when: discovery.state === 1 },
        State { name: "resolved";    when: discovery.state === 2 }
    ]

    // -------------------------
    // Idle
    // -------------------------
    Column {
        visible: root.state === "idle"
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Not discovering servers"
            color: "white"
            font.pixelSize: 22
        }

        Button {
            text: "Start Discovery"
            onClicked: discovery.startDiscovery()
        }
    }

    // -------------------------
    // Discovering
    // -------------------------
    Column {
        visible: root.state === "discovering"
        anchors.fill: parent
        spacing: 10
        padding: 20

        Text {
            text: "Searching for servers…"
            color: "white"
            font.pixelSize: 22
        }

        ListView {
            id: list
            model: root.model
            anchors.fill: parent
            anchors.topMargin: 50

            delegate: Rectangle {
                width: parent.width
                height: 60
                radius: 6
                color: selected ? "#405060" : "#303030"

                Text {
                    text: name + " (" + host + ")"
                    anchors.centerIn: parent
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.model.toggleSelection(index)
                }
            }
        }

        Row {
            spacing: 20

            Button {
                text: "Stop"
                onClicked: discovery.stopDiscovery()
            }

            Button {
                text: "Select"
                enabled: model.selectedServer().isValid
                onClicked: discovery.selectServer(model.selectedServer())
            }
        }
    }

    // -------------------------
    // Resolved
    // -------------------------
    Column {
        visible: root.state === "resolved"
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "Server ready to connect"
            color: "white"
            font.pixelSize: 22
        }

        Button {
            text: "Back to Results"
            onClicked: discovery.startDiscovery()
        }

        Button {
            text: "Restart Discovery"
            onClicked: discovery.restart()
        }
    }
}
