package com.lyect.placefinder.containers;

public class Location {
	private final String name;
	private final double latitude;
	private final double longitude;

	public Location() {
		name = "";
		latitude = 0;
		longitude = 0;
	}
	public Location(String _name, double _latitude, double _longitude) {
		name = _name;
		latitude = _latitude;
		longitude = _longitude;
	}

	public String getName() {
		return name;
	}

	public double getLatitude() {
		return latitude;
	}

	public double getLongitude() {
		return longitude;
	}

	@Override
	public String toString() {
		return getName();
	}
}
