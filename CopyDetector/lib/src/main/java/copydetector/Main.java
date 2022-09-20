package copydetector;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Main {

	public static void main(String[] args) throws Exception {
		
		if (args.length == 0) {
			throw new Exception("Not enough arguments!");
		}
		if (args.length > 1) {
			throw new Exception("Too much arguments!");
		}
		
		String multicastAddressString = args[0];
		
		CopyDetector copyDetector = new CopyDetector(multicastAddressString);
		
		System.out.println("Enter \"STOP\" when you want to stop");
		
		copyDetector.hello();
		copyDetector.listen();
		
		BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
		
		while (true) {
			String input = reader.readLine();
			
			if (input.equals("STOP")) {
				break;
			}
		}
		
		copyDetector.goodbye();
		
		reader.close();
		
	}

}
