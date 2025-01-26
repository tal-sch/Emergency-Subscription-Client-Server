package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Reactor;

import java.util.HashMap;
import bgu.spl.net.impl.stomp.ConnectionsImpl;

public class StompProtocol implements StompMessagingProtocol<Frame> {

    private int connectionId;
    private ConnectionsImpl<Frame> connections;
    private boolean terminate;

    @Override
    public void start(int connectionId, ConnectionsImpl<Frame> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
        this.terminate = false;
    }

    @Override
    public void process(Frame msg) {
        switch (msg.getCommand()) {
            case "CONNECT":
                handleConnect(msg);
                break;
            case "SEND":
                handleSend(msg);
                break;
            case "SUBSCRIBE":
                handleSubscribe(msg);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(msg);
                break;
            case "DISCONNECT":
                handleDisconnect(msg);
                break;
            default:
                handleUnknown(msg);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return terminate;
    }

    //Helper Methods for Frame Handling

    private void handleConnect(Frame msg) {
        String username = msg.getHeaders().get("login");
        String password = msg.getHeaders().get("passcode");
    
        // Missing username or password
        if (username == null || password == null) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing 'username' or 'password' header in CONNECT frame");
            Frame errorFrame = new Frame("ERROR", errorHeaders, "");
            connections.send(connectionId, errorFrame);
            return; 
        }
    
        // User already logged in
        if (connections.isUserActive(username)) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "User already logged in");
            Frame errorFrame = new Frame("ERROR", errorHeaders, "");
            connections.send(connectionId, errorFrame);
            return; 
        }
    
        // User exists but incorrect password
        if (connections.checkUsername(username) && !connections.getPasscode(username).equals(password)) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Incorrect passcode");
            Frame errorFrame = new Frame("ERROR", errorHeaders, "");
            connections.send(connectionId, errorFrame);
            return; 
        }
    
        // New user creation
        if (!connections.checkUsername(username)) {
            connections.addUserCredentials(username, password);
        }
    
        // Successful login
        Frame response = ProcessConnect(msg);
        if (response.getCommand().equals("ERROR")) {
            connections.send(connectionId, response);
        } else {
            connections.addActiveUser(connectionId, username);
            connections.send(connectionId, response);
        }
    }
    
    
    

    private void handleSend(Frame msg) {
        String destination = msg.getHeaders().get("destination");
    
        // Normalize the destination to remove leading `/`
        if (destination != null && destination.startsWith("/")) {
            destination = destination.substring(1);
        }
    
        if (destination == null || destination.isEmpty()) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing or empty 'destination' header");
            Frame errorFrame = new Frame("ERROR", errorHeaders, "");
            connections.send(connectionId, errorFrame);
            return;
        }
    
        if (!connections.isSubscribed(destination, connectionId)) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Not subscribed to the given topic");
            Frame errorFrame = new Frame("ERROR", errorHeaders, "");
            connections.send(connectionId, errorFrame);
            return;
        }
    
        Frame response = ProcessSend(msg);
        if (!response.getCommand().equals("ERROR")) {
            connections.send(destination, msg);
        } else {
            connections.send(connectionId, response);
        }
    }
    
    
    

    private void handleSubscribe(Frame msg) {
        Frame response = ProcessSubscribe(msg);
    
        if (response.getCommand().equals("ERROR")) {
            connections.send(connectionId, response);
        } else {
            String destination = msg.getHeaders().get("destination");
            String subscriptionId = msg.getHeaders().get("id");
            connections.subscribeToTopic(connectionId, destination, subscriptionId);
            String receiptId = msg.getHeaders().get("receipt");
            if (receiptId != null) {
                HashMap<String, String> receiptHeaders = new HashMap<>();
                receiptHeaders.put("receipt-id", receiptId);
                Frame receiptFrame = new Frame("RECEIPT", receiptHeaders, "");
                connections.send(connectionId, receiptFrame);
            }
        }
    }

    private void handleUnsubscribe(Frame msg) {
        Frame response = ProcessUnsubscribe(msg);
    
        if (response.getCommand().equals("ERROR")) {
            connections.send(connectionId, response);
        } else {
            String subscriptionId = msg.getHeaders().get("id");
            connections.unsubscribeFromTopic(connectionId, subscriptionId);
            String receiptId = msg.getHeaders().get("receipt");
            if (receiptId != null) {
                HashMap<String, String> receiptHeaders = new HashMap<>();
                receiptHeaders.put("receipt-id", receiptId);
                Frame receiptFrame = new Frame("RECEIPT", receiptHeaders, "");
                connections.send(connectionId, receiptFrame);
            }
        }
    }
    

    private void handleDisconnect(Frame msg) {
        Frame response = ProcessDisconnect(msg);
    
        if (response.getCommand().equals("ERROR")) {
            connections.send(connectionId, response);
        } 
        else {
            String receiptId = msg.getHeaders().get("receipt");
            if (receiptId != null) {
                HashMap<String, String> receiptHeaders = new HashMap<>();
                receiptHeaders.put("receipt-id", receiptId);
                Frame receiptFrame = new Frame("RECEIPT", receiptHeaders, "");
                connections.send(connectionId, receiptFrame);
            }
            connections.disconnect(connectionId);
            terminate = true;
        }
    }    

    private void handleUnknown(Frame msg) {
        HashMap<String, String> errorHeaders = new HashMap<>();
        errorHeaders.put("message", "Unsupported frame type: " + msg.getCommand());
        Frame errorFrame = new Frame("ERROR", errorHeaders, "");
        connections.send(connectionId, errorFrame);
    }

    private Frame ProcessConnect(Frame msg) {
        if (!msg.getHeaders().containsKey("accept-version") || !msg.getHeaders().get("accept-version").equals("1.2")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Invalid or missing 'accept-version' header in CONNECT frame");
            return new Frame("ERROR", errorHeaders, "");
        }
        if (!msg.getHeaders().containsKey("host") || !msg.getHeaders().get("host").equals("stomp.cs.bgu.ac.il")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Invalid or missing 'host' header in CONNECT frame");
            return new Frame("ERROR", errorHeaders, "");
        }
        if (!msg.getHeaders().containsKey("login")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing 'login' header in CONNECT frame");
            return new Frame("ERROR", errorHeaders, "");
        }
        if (!msg.getHeaders().containsKey("passcode")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing 'passcode' header in CONNECT frame");
            return new Frame("ERROR", errorHeaders, "");
        }

        HashMap<String, String> responseHeaders = new HashMap<>();
        responseHeaders.put("version", "1.2");
        return new Frame("CONNECTED",responseHeaders, "");

    }

    private Frame ProcessSend(Frame msg) {
        if (!msg.getHeaders().containsKey("destination")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Invalid or missing 'destination' header or more than one header in SEND frame");
            return new Frame("ERROR", errorHeaders, "");
        }

        return  msg;
    }

    private Frame ProcessSubscribe(Frame msg) {
        if (!msg.getHeaders().containsKey("destination")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing 'destination' header in SUBSCRIBE frame");
            return new Frame("ERROR", errorHeaders, "");
        }
    
        return msg;
    }

    private Frame ProcessUnsubscribe(Frame msg) {
        if (!msg.getHeaders().containsKey("id")) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Missing 'id' header in UNSUBSCRIBE frame");
            return new Frame("ERROR", errorHeaders, "");
        }
    
        return msg;
    }
    

    private Frame ProcessDisconnect(Frame msg) {
    
        if (msg.getHeaders().containsKey("receipt") && msg.getHeaders().get("receipt").isEmpty()) {
            HashMap<String, String> errorHeaders = new HashMap<>();
            errorHeaders.put("message", "Empty 'receipt' header in DISCONNECT frame");
            return new Frame("ERROR", errorHeaders, "");
        }
    
        return msg;
    }
    
    
}
