package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.StringJoiner;

public class Frame {


    private String command;
    private HashMap<String, String> headers;
    private String body;

    public Frame(String command, HashMap<String, String> headers, String body) {
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    public String getCommand() {
        return command;
    }

    public HashMap<String, String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    public void setCommand(String command){
        this.command = command;
    }

    public void setHeaders(HashMap<String, String> headers){
        this.headers = headers;
    }

    @Override
    public String toString() {
        StringJoiner frameFormat = new StringJoiner("\n");
        frameFormat.add(command);
        for (HashMap.Entry<String, String> entry : headers.entrySet()) {
            frameFormat.add(entry.getKey() + ":" + entry.getValue());
        }
        frameFormat.add("").add(body);
        return frameFormat.toString() + '\0';
    }



    public static Frame parse(String rawFrame) {
        String[] parts = rawFrame.split("\n\n", 2); // Split into headers+command and body
        String headerPart = parts[0];
        String body = parts.length > 1 ? parts[1].replace("\0", "") : "";

        String[] lines = headerPart.split("\n");
        String command = lines[0];
        HashMap<String, String> headers = new HashMap<>();

        // Parse headers
        for (int i = 1; i < lines.length; i++) {
            String[] keyValue = lines[i].split(":", 2);
            if (keyValue.length == 2) {
                headers.put(keyValue[0].trim(), keyValue[1].trim());
            }
        }
        return new Frame(command, headers, body);
    }


}
