package copydetector;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashMap;

public class CopyDetector {
	private String multicastAddressString;
	private MulticastSocket multicastSocket;
	private Thread listener;
	
	private int nonce;
	private final int KEY = 228;
	
	private int MTC(int N) {
		int hashPart = Integer.hashCode(N);
		int hashFull = Integer.hashCode(hashPart + KEY);
		return hashFull;
	}
	
	public CopyDetector(String _multicastAddressString) throws IOException {
		multicastAddressString = _multicastAddressString;
		multicastSocket = new MulticastSocket(8000);
		multicastSocket.setSoTimeout(1);
		multicastSocket.joinGroup(InetAddress.getByName(multicastAddressString));
		
		nonce = (int) (Math.random() * 100);
	}
	
	private DatagramPacket makePacket(int N) throws UnknownHostException {
		ByteBuffer byteContent = ByteBuffer.allocate(8);
		
		byteContent.putInt(MTC(N));
		byteContent.putInt(N);
		
		DatagramPacket datagram = new DatagramPacket(
			byteContent.array(),
			byteContent.array().length,
			InetAddress.getByName(multicastAddressString),
			8000
		);
		
		return datagram;
	}
	
	public void hello() throws IOException {
		DatagramPacket datagram = makePacket(nonce);
		multicastSocket.send(datagram);
		
		nonce += 1;
	}
	
	public void listen() {
		
		listener = new Thread() {
			
			private void printHostsList(HashMap<String, Integer> knownHosts) {
				System.out.println("List of online hosts has changed:");
				for (String host : knownHosts.keySet()) {
					System.out.println(host);
				}
			}
			
			public void run() {
				
				HashMap<String, Integer> knownHosts = new HashMap<>();
				
				while (true) {
					if (isInterrupted()) {
						break;
					}
					
					DatagramPacket receivedDatagram = new DatagramPacket(new byte[400], 400);
					
					try {
						multicastSocket.receive(receivedDatagram);
					} catch (IOException e) {
						continue;
					}
					
					String sourceAddress = receivedDatagram.getAddress().getHostAddress();
					ByteBuffer byteContent = ByteBuffer.wrap(receivedDatagram.getData());
					
					int receivedHash = byteContent.getInt();
					int receivedNonce = byteContent.getInt();
					
					if (receivedHash == MTC(receivedNonce)) {
						if (!knownHosts.containsKey(sourceAddress)) {
							System.out.println(sourceAddress + " online.");
							knownHosts.put(sourceAddress, receivedNonce);
							printHostsList(knownHosts);
							
							try {
								DatagramPacket repeatPacket = makePacket(nonce - 1);
								multicastSocket.send(repeatPacket);
							}
							catch (Exception e) {}
							
						}
						else {
							
							int storedNonce = knownHosts.get(sourceAddress);
							
							if (receivedNonce == storedNonce + 1) {
								System.out.println(sourceAddress + " offline.");
								knownHosts.remove(sourceAddress);
								printHostsList(knownHosts);
							}
						}
					}
				}
			}
		};
		
		listener.start();
	}
	
	public void goodbye() throws IOException, InterruptedException {
		listener.interrupt();
		listener.join();
		DatagramPacket datagram = makePacket(nonce);
		multicastSocket.send(datagram);
		multicastSocket.leaveGroup(InetAddress.getByName(multicastAddressString));
	}
}
