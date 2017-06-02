/*
  ==============================================================================

    SofaPathSharedUpdater.h
    Created: 1 Jun 2017 1:04:05pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

/** A singleton pattern style class for distributing a new loaded SOFA file path to all instances of the plugin via the InterprocessConnection class */
class SofaPathSharedUpdater
{
public:
    /** Singleton "Constructor" */
    static SofaPathSharedUpdater& instance(){
        static SofaPathSharedUpdater _instance;
        return _instance;
    }
    
    /** Creates a new unique connection and returns the pipename, which can be used to connect via connectToPipe(). Connections created this way  should be removed by removeConnection(String _id) */
    String createConnection(){
        String connection_id = String("SOFAPAN." + String(count_id));
        count_id++;
        connections.add(new Connection(connection_id, *this));
        return connection_id;
    }
    
    /** Removes a connection that hast been created by createConnection(). The _id has to be the exact String that was returned by createConnection() */
    void removeConnection(String _id){
        for(int i = 0; i < connections.size(); i++){
            if(_id == connections[i]->getConnectionId())
                connections.remove(i);
        }
    }
    
    /** Returns the last path that has been broadcastet, e.g. for new plugin instances */
    String requestCurrentFilePath(){
        return currentFilePath;
    }
    
private:
    class Connection: public InterprocessConnection{
    private:
        void connectionMade() override{
        }
        void connectionLost() override{
        }
        void messageReceived (const MemoryBlock &message) override{
            updater.broadcastMessage(message, getPipe()->getName());
        }
        SofaPathSharedUpdater& updater;
    public:
        Connection(String _id, SofaPathSharedUpdater& u)
        :updater(u)
        {
            createPipe(_id, 10);
        }
        ~Connection(){
            disconnect();
        }
        String getConnectionId(){
            return getPipe()->getName();
        }
    };
    void broadcastMessage(const MemoryBlock &message, String sender){
        for(int i = 0; i < connections.size(); i++){
            if(connections[i]->getConnectionId() != sender)
                connections[i]->sendMessage(message);
        }
        currentFilePath = message.toString();
    }
    
    SofaPathSharedUpdater(){}
    SofaPathSharedUpdater(const SofaPathSharedUpdater&);
    void operator=(SofaPathSharedUpdater const&);
    ~SofaPathSharedUpdater(){}
    OwnedArray<Connection> connections;
    int count_id = 1000;
    String currentFilePath = "";
};
