package com.lyect.placefinder.controllers.infocontrollers;


import com.lyect.placefinder.containers.LocationInfo;
import javafx.application.Platform;
import javafx.scene.control.Label;

public class LocationInfoController implements InfoController<LocationInfo> {

	private final Label selectedLocation;
	private final Label temperatureLocation;
	private final Label cloudsLocation;

	public LocationInfoController(
			Label _selectedLocation,
			Label _temperatureLocation,
			Label _cloudsLocation
	) {
		selectedLocation = _selectedLocation;
		temperatureLocation = _temperatureLocation;
		cloudsLocation = _cloudsLocation;
	}


	@Override
	public void block() {
		// Location data should not be blocked
		// It is just a text and not an interactive element
	}

	@Override
	public void unblock() {
		// If it never blocks, it will not need to be unblocked
	}

	@Override
	public void clear() {
		Platform.runLater(() -> {
			selectedLocation.setText("UNKNOWN");
			temperatureLocation.setText("UNKNOWN");
			cloudsLocation.setText("UNKNOWN");
		});
	}

	@Override
	public void update(LocationInfo newLocationInfo) {
		Platform.runLater(() -> {
			selectedLocation.setText(newLocationInfo.getLocationName());
			temperatureLocation.setText(newLocationInfo.getTemperature());
			cloudsLocation.setText(newLocationInfo.getClouds());
		});
	}
}
