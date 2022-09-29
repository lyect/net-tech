package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class CopyDetector {
	
	private Thread senderThread;	// sends packets
	private Thread listenerThread;	// listens for incoming packets and answers on them
	
	private MulticastSocket multicastSocket;
	private String multicastAddressString;
	
	private BlockingQueue<DatagramPacket> messageQueue;
	private final int QUEUE_CAPACITY = 10;
	private final boolean QUEUE_FAIRNESS = true;
	
	private int nonce;
	private final int KEY = 228;
	
	private final int OUTDATING_SECONDS = 10;
	
	public CopyDetector(String _multicastAddressString) throws IOException {
		multicastAddressString = _multicastAddressString;
		
		multicastSocket = new MulticastSocket(8000);
		multicastSocket.setSoTimeout(100);
		
		messageQueue = new ArrayBlockingQueue<DatagramPacket>(QUEUE_CAPACITY, QUEUE_FAIRNESS);
		
		PacketMaker.multicastAddressString = multicastAddressString;
		PacketMaker.key = KEY;
	}
	
	private void hello() throws UnknownHostException, InterruptedException {
		DatagramPacket helloPacket = PacketMaker.makePacket(nonce);
		messageQueue.put(helloPacket);
		nonce += 1;
	}
	
	private void goodbye() throws UnknownHostException, InterruptedException {
		DatagramPacket goodbyePacket = PacketMaker.makePacket(nonce);
		messageQueue.put(goodbyePacket);
	}
	
	public void start() throws IOException, InterruptedException {
		multicastSocket.joinGroup(InetAddress.getByName(multicastAddressString));
		
		nonce = (int) (Math.random() * 100);
		
		Sender sender = new Sender(multicastSocket, messageQueue, nonce, OUTDATING_SECONDS / 2);
		Listener listener = new Listener(multicastSocket, messageQueue, nonce, KEY, OUTDATING_SECONDS);
		
		senderThread = new Thread(sender);
		listenerThread = new Thread(listener);
		
		senderThread.start();
		listenerThread.start();
		
		hello();
	}
	
	public void stop() throws IOException, InterruptedException {
		listenerThread.interrupt();
		goodbye();
		senderThread.interrupt();
	}
}
