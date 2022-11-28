package com.lyect.placefinder.controllers.choicecontrollers;

import com.lyect.placefinder.containers.Location;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.control.ChoiceBox;

import java.util.List;

public class LocationChoiceController implements ChoiceController<Location> {

	private final ChoiceBox<Location> locationChoice;
	private final ObservableList<Location> locationsList;
	private final Object blockingSynchronization = new Object();

	public LocationChoiceController(ChoiceBox<Location> _locationChoice) {
		locationChoice = _locationChoice;
		locationsList = FXCollections.observableArrayList();
		Platform.runLater(() -> {
			locationChoice.setItems(locationsList);
		});
	}

	@Override
	public void block() {
		Platform.runLater(() -> {
			synchronized (blockingSynchronization) {
				locationChoice.setDisable(true);
			}
		});
	}

	@Override
	public void unblock() {
		Platform.runLater(() -> {
			synchronized (blockingSynchronization) {
				locationChoice.setDisable(false);
			}
		});
	}

	@Override
	public void clear() {
		Platform.runLater(locationsList::clear);
	}

	@Override
	public void update(List<Location> newLocations) {
		Platform.runLater(() -> {
			locationsList.setAll(newLocations);
		});
	}
}
