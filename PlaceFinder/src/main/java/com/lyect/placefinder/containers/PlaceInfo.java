package com.lyect.placefinder.containers;

public class PlaceInfo {
	private final String placeName;
	private final String placeInfo;

	public PlaceInfo() {
		placeName = "";
		placeInfo = "";
	}

	public PlaceInfo(String _placeName, String _placeInfo) {
		placeName = _placeName;
		placeInfo = _placeInfo;
	}

	public String getPlaceName() {
		return placeName;
	}

	public String getPlaceInfo() {
		return placeInfo;
	}
}
