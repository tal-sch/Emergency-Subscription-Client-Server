package bgu.spl.net.api;

import bgu.spl.net.impl.stomp.ConnectionsImpl;
import bgu.spl.net.srv.Connections;

public interface MessagingProtocol<T> {
    
    void start(int connectionId, ConnectionsImpl<T> connections);
    /**
     * process the given message 
     * @param msg the received message
     * @return the response to send or null if no response is expected by the client
     */
    void process(T msg);
 
    /**
     * @return true if the connection should be terminated
     */
    boolean shouldTerminate();
 
}