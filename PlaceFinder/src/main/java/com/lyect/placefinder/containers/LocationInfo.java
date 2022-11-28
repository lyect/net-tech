package com.lyect.placefinder.containers;

public class LocationInfo {
	private String locationName;
	private final String temperature;
	private final String clouds;

	public LocationInfo(){
		locationName = "";
		temperature = "";
		clouds = "";
	}
	public LocationInfo(String _locationName, String _temperature, String _clouds) {
		locationName = _locationName;
		temperature = _temperature;
		clouds = _clouds;
	}

	public String getLocationName() {
		return locationName;
	}

	public String getTemperature() {
		return temperature;
	}

	public String getClouds() {
		return clouds;
	}

	public LocationInfo setLocationNameReturn(String newName) {
		locationName = newName;
		return this;
	}
}
