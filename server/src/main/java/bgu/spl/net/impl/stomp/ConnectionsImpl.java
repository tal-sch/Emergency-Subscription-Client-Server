package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class ConnectionsImpl <T> implements Connections <T>{

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> connectionHandlers = new ConcurrentHashMap<>();
    private final Map<Integer, Map<String, String>> userSubscriptions = new HashMap<>(); // connectionId -> (topic -> subscriptionId)
    private final ConcurrentHashMap<String, String> userCredentials = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<Integer, String> activeUsers = new ConcurrentHashMap<>();
    private final AtomicInteger messageIdCounter = new AtomicInteger(0);

    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = connectionHandlers.get(connectionId);
        if (handler != null) {
            handler.send(msg);
            return true;
        }
        return false;
    }
    
    public void send(String channel, T msg) {
        if (!(msg instanceof Frame)) {
            throw new IllegalArgumentException("Message must be of type Frame");
        }
    
        Frame originalFrame = (Frame) msg;
    
        for (Map.Entry<Integer, Map<String, String>> entry : userSubscriptions.entrySet()) {
            Integer connectionId = entry.getKey();
            Map<String, String> subscriptions = entry.getValue();
    
            if (subscriptions.containsKey(channel)) {
                String subscriptionId = subscriptions.get(channel); // Client-specific subscriptionId
                HashMap<String, String> messageHeaders = new HashMap<>(originalFrame.getHeaders());
                messageHeaders.put("subscription", subscriptionId); // Add subscriptionId for the client
                messageHeaders.put("message-id", String.valueOf(messageIdCounter.getAndIncrement()));
                messageHeaders.put("destination", "/" + channel);
    
                Frame messageFrame = new Frame("MESSAGE", messageHeaders, originalFrame.getBody());
                send(connectionId, (T) messageFrame); // Send to the client
            }
        }
    }
    
    
    
    
    

    public void disconnect(int connectionId) {
        userSubscriptions.remove(connectionId);
        connectionHandlers.remove(connectionId);
        removeActiveUser(connectionId);
    }
    

    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        connectionHandlers.put(connectionId, handler);
    }

    public void subscribeToTopic(int connectionId, String topic, String subscriptionId) {
        userSubscriptions.computeIfAbsent(connectionId, k -> new HashMap<>());
        userSubscriptions.get(connectionId).put(topic, subscriptionId);
    }
    

    public void unsubscribeFromTopic(int connectionId, String topic) {
        Map<String, String> subscriptions = userSubscriptions.get(connectionId);
        if (subscriptions != null) {
            subscriptions.remove(topic);
            if (subscriptions.isEmpty()) {
                userSubscriptions.remove(connectionId);
            }
        }
    }
    

    public void addUserCredentials(String username, String password) {
        userCredentials.put(username, password);
    }

    public boolean checkUsername(String username) {
        return userCredentials.containsKey(username);
    }

    public String getPasscode(String username){
        return userCredentials.get(username);
    }

    public void addActiveUser(int connectionId, String username) {
        activeUsers.put(connectionId, username);
    }

    public void removeActiveUser(Integer connectionId) {
        activeUsers.remove(connectionId);
    }

    public boolean isUserActive(String username) {
        return activeUsers.values().contains(username);
    }
    public ConcurrentHashMap<Integer, String> getActiveUsers() {
        return activeUsers;
    }

    public ConnectionHandler<T> getHandler(int connectionId) {
        return connectionHandlers.get(connectionId);
    }

    public boolean isSubscribed(String topic, int connectionId) {
        Map<String, String> subscriptions = userSubscriptions.get(connectionId);
        return subscriptions != null && subscriptions.containsKey(topic);
    }

}
