package copydetector;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Main {

	public static void main(String[] args) throws Exception {
		
		if (args.length == 0) {
			throw new Exception("Multicast address has not been passed! Provide it as a program argument.");
		}
		if (args.length > 1) {
			throw new Exception("Too much arguments! There is must be only one: multicast address.");
		}
		
		String multicastAddressString = args[0];
		
		CopyDetector copyDetector = new CopyDetector(multicastAddressString);
		
		System.out.println("Enter \"STOP\" when you want to stop");
		
		copyDetector.start();
		
		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
		while (true) {
			String input = reader.readLine();
			
			if (input.equals("STOP")) {
				break;
			}
		}
		
		copyDetector.stop();
		
		reader.close();
	}

}
