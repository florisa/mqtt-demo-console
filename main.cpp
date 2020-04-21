#include <QCoreApplication>
#include<QtMqtt/QtMqtt>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QMqttClient *client = new QMqttClient();
    client->setProtocolVersion(QMqttClient::MQTT_5_0);
    client->setHostname("localhost");
    client->setPort(1883);

    a.connect(client, &QMqttClient::connected,[&client](){
        qDebug() << "Connected:";
        const auto serverProp = client->serverConnectionProperties();
        const auto available = serverProp.availableProperties();
        if(available & QMqttServerConnectionProperties::MaximumPacketSize)
            qDebug() << "Max Packet Size" << serverProp.maximumPacketSize();
        if(available & QMqttServerConnectionProperties::MaximumTopicAlias)
            qDebug() <<"Max Topic Alias:" << serverProp.maximumTopicAlias();

        QMqttSubscriptionProperties subProps;
        subProps.setSubscriptionIdentifier(42);
        auto sub = client->subscribe(QLatin1String("Topics/some"),subProps,1);

        sub->connect(sub, &QMqttSubscription::messageReceived, [](const QMqttMessage &msg)
        {
            qDebug() << "Message received:" << msg.payload();
            qDebug() << "Topic Alias:"<< msg.publishProperties().topicAlias();
            qDebug() << "Response Topic" << msg.publishProperties().responseTopic();
        });

        client->connect(sub, &QMqttSubscription::stateChanged, [&client](QMqttSubscription::SubscriptionState s)
        {
            qDebug() << "Subscription new State:" << s;
            if(s == QMqttSubscription::Subscribed)
            {
                qDebug() << "Subscription established.";

                QMqttPublishProperties pubProps;
                pubProps.setResponseTopic(QLatin1String("Please/reply/here"));
                pubProps.setTopicAlias(1);
                client->publish(QLatin1String("topic/some"),pubProps,"foo",1);
            }

        });

    });

    QMqttConnectionProperties conProp;
    conProp.setMaximumTopicAlias(5);
    client->setConnectionProperties(conProp);
    client->connectToHost();


    return a.exec();
}
