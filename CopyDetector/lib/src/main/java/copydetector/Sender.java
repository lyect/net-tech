package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.MulticastSocket;
import java.util.concurrent.BlockingQueue;

public class Sender implements Runnable {
	private BlockingQueue<DatagramPacket> messageQueue;
	private MulticastSocket multicastSocket;
	
	public Sender(MulticastSocket _multicastSocket, BlockingQueue<DatagramPacket> _messageQueue) {
		messageQueue = _messageQueue;
		multicastSocket = _multicastSocket;
	}
	
	@Override
	public void run() {
		while (true) {
			
			if (Thread.currentThread().isInterrupted()) {
				break;
			}
			
			try {
				DatagramPacket message = messageQueue.take();
				multicastSocket.send(message);
			} catch (InterruptedException e) {
				break;
			} catch (IOException e) {
				// Don't care
			}
		}
	}
}
