package com.lyect.placefinder.requester;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.lyect.placefinder.containers.Location;
import com.lyect.placefinder.containers.LocationInfo;
import com.lyect.placefinder.containers.Place;
import com.lyect.placefinder.containers.PlaceInfo;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;

public class JSONParser {

	private static <T> Predicate<T> distinctByKey(Function<? super T, ?> keyExtractor) {
		Set<Object> seen = ConcurrentHashMap.newKeySet();
		return t -> seen.add(keyExtractor.apply(t));
	}

	private static final ObjectMapper objectMapper = new ObjectMapper();
	public static List<Location> parseFoundLocationsJSON(String responseBodyJSON) {
		try {

			JsonNode locationArrayNode = objectMapper.readTree(responseBodyJSON).get("hits");
			Iterator<JsonNode> locationIterator = locationArrayNode.elements();
			Iterable<JsonNode> locationIterable = () -> locationIterator;

			return StreamSupport.stream(locationIterable.spliterator(), false)
					.map(locationNode -> new Location(
							locationNode.get("name").asText() + ", " + locationNode.get("country").asText(),
							Double.parseDouble(locationNode.get("point").get("lat").asText()),
							Double.parseDouble(locationNode.get("point").get("lng").asText())
					))
					.filter(distinctByKey(Location::getName))
					.filter(location -> !location.getName().equals(""))
					.collect(Collectors.toList());
		} catch (JsonProcessingException e) {
			return new ArrayList<Location>();
		}
	}

	public static LocationInfo parseLocationInfoJSON(String responseBodyJSON) {
		try {

			JsonNode rootNode = objectMapper.readTree(responseBodyJSON);
			JsonNode weatherNode = rootNode.get("weather");
			JsonNode temperatureNode = rootNode.get("main").get("temp");

			Iterator<JsonNode> weatherFeatureIterator = weatherNode.elements();
			JsonNode cloudsNode = weatherFeatureIterator.next().get("main");

			return new LocationInfo(
					"",
					String.format("%.2f", Double.parseDouble(temperatureNode.asText()) - 273.15),
					cloudsNode.asText()
			);
		} catch (JsonProcessingException e) {
			return new LocationInfo();
		}
	}

	public static List<Place> parseFoundPlacesJSON(String responseBodyJSON) {
		try {
			JsonNode placesNode = objectMapper.readTree(responseBodyJSON).get("features");

			Iterator<JsonNode> placeIterator = placesNode.elements();
			Iterable<JsonNode> placeIterable = () -> placeIterator;

			return StreamSupport.stream(placeIterable.spliterator(), false)
					.map(placeNode -> new Place(
							placeNode.get("properties").get("name").asText(),
							placeNode.get("properties").get("xid").asText()
					))
					.filter(distinctByKey(Place::getName))
					.filter(place -> !place.getName().equals(""))
					.collect(Collectors.toList());
		} catch (JsonProcessingException e) {
			return new ArrayList<Place>();
		}
	}

	public static PlaceInfo parsePlaceInfoJSON(String responseBodyJSON) {
		try {
			JsonNode placeInfoNode = objectMapper.readTree(responseBodyJSON);

			JsonNode wikiExtractNode = placeInfoNode.findValue("wikipedia_extracts");

			if (wikiExtractNode != null) {
				return new PlaceInfo(
						placeInfoNode.get("name").asText(),
						wikiExtractNode.get("text").asText()
				);
			}
			else {
				return new PlaceInfo(
						placeInfoNode.get("name").asText(),
						""
				);
			}
		} catch (JsonProcessingException e) {
			return new PlaceInfo();
		}
	}
}
