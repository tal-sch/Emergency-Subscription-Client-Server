package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

import java.util.InputMismatchException;
import java.util.concurrent.atomic.AtomicInteger;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            throw new IllegalArgumentException("Usage: StompServer <port> <tpc/reactor>");
        }

        int port;
        try {
            port = Integer.parseInt(args[0]);
            if (port < 1 || port > 65535) {
                throw new IllegalArgumentException("Port must be between 1 and 65535.");
            }
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Port must be a valid number.");
        }


        if (args[1].equals("tpc")) {
            Server.threadPerClient(
                port,
                StompProtocol::new,
                FrameEncoderDecoder::new
            ).serve();
        } else if (args[1].equals("reactor")) {
            Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                port,
                StompProtocol::new,
                FrameEncoderDecoder::new
            ).serve();
        } else {
            throw new InputMismatchException("Invalid mode. Use 'tpc' or 'reactor'.");
        }
    }
}
