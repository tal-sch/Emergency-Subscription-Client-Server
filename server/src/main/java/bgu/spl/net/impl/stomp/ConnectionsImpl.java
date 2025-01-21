package bgu.spl.net.impl.stomp;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl <T> implements Connections <T>{

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> connectionHandlers = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String, Set<Integer>> topicSubscribers = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<String, String> userCredentials = new ConcurrentHashMap<>();
    private final ConcurrentHashMap<Integer, String> activeUsers = new ConcurrentHashMap<>();

    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = connectionHandlers.get(connectionId);
        System.out.println(handler == null);
        if (handler != null) {
            handler.send(msg);
            System.out.println("handler sent");
            return true;
        }
        return false;
    }
    
    public void send(String channel, T msg) {
        Set<Integer> subscribers = topicSubscribers.get(channel);
        if (subscribers != null) {
            for (Integer connectionId : subscribers) {
                send(connectionId, msg);
            }
        }
    }
    

    public void disconnect(int connectionId) {
        for (Set<Integer> subscribers : topicSubscribers.values()) {
            subscribers.remove(connectionId);
        }

        removeActiveUser(connectionId);
    }
    

    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        connectionHandlers.put(connectionId, handler);
    }

    public void subscribeToTopic(int connectionId, String topic) {
        topicSubscribers.putIfAbsent(topic, new HashSet<>());
        topicSubscribers.get(topic).add(connectionId);
    }

    public void unsubscribeFromTopic(int connectionId, String topic) {
        Set<Integer> subscribers = topicSubscribers.get(topic);
        if (subscribers != null) {
            subscribers.remove(connectionId);
            if (subscribers.isEmpty()) {
                topicSubscribers.remove(topic);
            }
        }
    }

    public void addUserCredentials(String username, String password) {
        userCredentials.put(username, password);
    }

    public boolean authenticateUser(String username, String password) {
        return userCredentials.getOrDefault(username, "").equals(password);
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

}
