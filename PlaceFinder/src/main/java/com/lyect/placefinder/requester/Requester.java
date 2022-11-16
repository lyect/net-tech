package com.lyect.placefinder.requester;

import com.lyect.placefinder.containers.*;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.List;
import java.util.concurrent.CompletableFuture;

public class Requester {

	private static final String FIND_LOCATION_REQUEST_URL = "https://graphhopper.com/api/1/geocode";
	private static final String LOCATION_INFO_REQUEST_URL = "https://api.openweathermap.org/data/2.5/weather";
	private static final String FIND_PLACES_REQUEST_URL = "http://api.opentripmap.com/0.1/ru/places/radius";
	private static final String PLACE_INFO_REQUEST_URL = "http://api.opentripmap.com/0.1/ru/places/xid";
	
	private static final String FIND_PLACES_RADIUS = "1000"; // 1 km

	private static final HttpClient httpClient = HttpClient.newHttpClient();

	public static CompletableFuture<List<Location>> makeFindLocationRequest(String stringLocation) {
		String requestURIString = String.format("%s?q=%s&locale=%s&key=%s",
				FIND_LOCATION_REQUEST_URL,
				stringLocation,
				"us",
				KeyUtils.FIND_LOCATION_KEY
		);

		HttpRequest request = HttpRequest.newBuilder()
				.GET()
				.uri(URI.create(requestURIString))
				.build();

		return httpClient.sendAsync(request, HttpResponse.BodyHandlers.ofString())
				.thenApply(HttpResponse::body)
				.thenApply(JSONParser::parseFoundLocationsJSON);
	}

	public static CompletableFuture<LocationInfo> makeLocationInfoRequest(Location selectedLocation) {
		String requestURIString = String.format("%s?lat=%f&lon=%f&appid=%s",
				LOCATION_INFO_REQUEST_URL,
				selectedLocation.getLatitude(),
				selectedLocation.getLongitude(),
				KeyUtils.LOCATION_INFO_KEY
		);

		HttpRequest request = HttpRequest.newBuilder()
				.GET()
				.uri(URI.create(requestURIString))
				.build();

		return httpClient.sendAsync(request, HttpResponse.BodyHandlers.ofString())
				.thenApply(HttpResponse::body)
				.thenApply(JSONParser::parseLocationInfoJSON)
				.thenApply(locationInfo -> locationInfo.setLocationNameReturn(selectedLocation.getName()));
	}

	public static CompletableFuture<List<Place>> makeFindPlacesRequest(Location selectedLocation) {
		String requestURIString = String.format("%s?lang=%s&lon=%s&lat=%s&radius=%s&apikey=%s",
				FIND_PLACES_REQUEST_URL,
				"en",
				selectedLocation.getLongitude(),
				selectedLocation.getLatitude(),
				FIND_PLACES_RADIUS,
				KeyUtils.FIND_PLACES_KEY
		);

		HttpRequest request = HttpRequest.newBuilder()
				.GET()
				.uri(URI.create(requestURIString))
				.build();

		return httpClient.sendAsync(request, HttpResponse.BodyHandlers.ofString())
				.thenApply(HttpResponse::body)
				.thenApply(JSONParser::parseFoundPlacesJSON);
	}

	public static CompletableFuture<PlaceInfo> makePlaceInfoRequest(Place selectedPlace) {
		String requestURIString = String.format("%s/%s?apikey=%s",
				PLACE_INFO_REQUEST_URL,
				selectedPlace.getXid(),
				KeyUtils.PLACE_INFO_KEY
		);

		HttpRequest request = HttpRequest.newBuilder()
				.GET()
				.uri(URI.create(requestURIString))
				.build();

		return httpClient.sendAsync(request, HttpResponse.BodyHandlers.ofString())
				.thenApply(HttpResponse::body)
				.thenApply(JSONParser::parsePlaceInfoJSON);
	}

}
