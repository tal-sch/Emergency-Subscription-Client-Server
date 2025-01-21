package bgu.spl.net.impl.stomp;
import java.io.Serializable;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import bgu.spl.net.api.MessageEncoderDecoder;

public class FrameEncoderDecoder implements MessageEncoderDecoder<Frame> {

    private final List<Byte> bytes = new ArrayList<>();

    @Override
    public Frame decodeNextByte(byte nextByte) {
        if (nextByte == '\0') {
            byte[] byteArray = new byte[bytes.size()];
            for (int i = 0; i < bytes.size(); i++) {
                byteArray[i] = bytes.get(i);
            }
            bytes.clear();
            String message = new String(byteArray, StandardCharsets.UTF_8);
            return Frame.parse(message);
        } else {
            bytes.add(nextByte);
            return null;
        }
    }

    public byte[] encode(Frame message) {
        return (message.toString()).getBytes(StandardCharsets.UTF_8);
    }

}