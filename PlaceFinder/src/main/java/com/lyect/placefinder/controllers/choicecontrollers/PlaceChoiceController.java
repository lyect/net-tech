package com.lyect.placefinder.controllers.choicecontrollers;

import com.lyect.placefinder.containers.Place;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.control.ChoiceBox;

import java.util.List;

public class PlaceChoiceController implements ChoiceController<Place> {

	private final ChoiceBox<Place> placeChoice;
	private final ObservableList<Place> placesList;
	private final Object blockingSynchronization = new Object();

	public PlaceChoiceController(ChoiceBox<Place> _placeChoice) {
		placeChoice = _placeChoice;
		placesList = FXCollections.observableArrayList();
		Platform.runLater(() -> {
			placeChoice.setItems(placesList);
		});
	}

	@Override
	public void block() {
		Platform.runLater(() -> {
			synchronized (blockingSynchronization) {
				placeChoice.setDisable(true);
			}
		});
	}

	@Override
	public void unblock() {
		Platform.runLater(() -> {
			synchronized (blockingSynchronization) {
				placeChoice.setDisable(false);
			}
		});
	}

	@Override
	public void clear() {
		Platform.runLater(placesList::clear);
	}

	@Override
	public void update(List<Place> newPlaces) {
		Platform.runLater(() -> {
			placesList.setAll(newPlaces);
		});
	}
}
