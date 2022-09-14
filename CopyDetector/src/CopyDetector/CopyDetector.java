package CopyDetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashMap;

public class CopyDetector {

	private String multicastAddress;
	private MulticastSocket socket;
	private int nonce;

	private static final int KEY = 228;
	private static final int PORT = 8000;

	private Thread listener;

	// should be something stronger
	private int MAC(int n) {
		int hashPart = Integer.hashCode(n);
		int hashFull = Integer.hashCode(hashPart + KEY);
		return hashFull;
	}

	// init multicast socket and nonce
	// then join to group
	public void connect(String _multicastAddress) throws IOException {
		multicastAddress = _multicastAddress;

		socket = new MulticastSocket(PORT);
		socket.setSoTimeout(1);
		socket.joinGroup(InetAddress.getByName(multicastAddress));

		nonce = (int) (Math.random() * 100);
	}
	
	private DatagramPacket makePacket(int n) throws UnknownHostException {
		int hashNumber = MAC(n);

		ByteBuffer datagramContent = ByteBuffer.allocate(8); // contains hashNumber and nonce
		datagramContent.putInt(hashNumber);
		datagramContent.putInt(n);

		DatagramPacket datagram = new DatagramPacket(
			datagramContent.array(),
			datagramContent.array().length,
			InetAddress.getByName(multicastAddress),
			PORT
		);
		
		return datagram;
	}
	
	public void hello() throws IOException {
		socket.send(makePacket(nonce));
		nonce += 1;
	}

	public void listen() {
		listener = new Thread() {
			public void run() {

				HashMap<String, Integer> authCopies = new HashMap<>();

				DatagramPacket receivedMessage = new DatagramPacket(new byte[400], 400);
				
				boolean copyListIsPrinted = true;
				
				while (true) {

					if (currentThread().isInterrupted()) {
						break;
					}

					try {
						socket.receive(receivedMessage);
					} catch (IOException e) {
						continue;
					}

					String stringSourceAddress = receivedMessage.getAddress().getHostAddress();
					ByteBuffer receivedMessageBufferWrapper = ByteBuffer.wrap(receivedMessage.getData());

					int receivedHash = receivedMessageBufferWrapper.getInt();
					int receivedNonce = receivedMessageBufferWrapper.getInt();

					if (MAC(receivedNonce) == receivedHash) {
						if (!authCopies.containsKey(stringSourceAddress)) {
							authCopies.put(stringSourceAddress, receivedNonce);
							System.out.println("Found copy at " + stringSourceAddress);
							// for every new copy resend hello packet of this app
							// this makes previously joined apps see who is online
							try {
								socket.send(makePacket(nonce - 1));
							} catch (IOException e) {
								System.out.println("Can not send replay packet");
							}
							copyListIsPrinted = false;
						}
						else {
							if (authCopies.get(stringSourceAddress) < receivedNonce) {
								authCopies.remove(stringSourceAddress);
								copyListIsPrinted = false;
								System.out.println("Copy at " + stringSourceAddress + " is turned off");
							}
						}
					}
					
					if (!copyListIsPrinted) {
						System.out.println("Number of copies changed. List of copies:");
						for (String copyAddress : authCopies.keySet()) {
							System.out.println("\t" + copyAddress);
						}
						copyListIsPrinted = true;
					}

				}
			}
		};

		listener.start();
	}

	public void goodbye() throws IOException, InterruptedException {
		
		listener.interrupt();
		listener.join();
		
		socket.send(makePacket(nonce));
		socket.close();
		
	}
}
