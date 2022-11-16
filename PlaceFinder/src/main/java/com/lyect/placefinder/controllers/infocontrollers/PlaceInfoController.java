package com.lyect.placefinder.controllers.infocontrollers;


import com.lyect.placefinder.containers.PlaceInfo;
import javafx.application.Platform;
import javafx.scene.control.Label;
import javafx.scene.text.Text;
import javafx.scene.text.TextFlow;

public class PlaceInfoController implements InfoController<PlaceInfo> {

	private final Label selectedPlace;
	private final Text infoPlace;

	public PlaceInfoController(
			Label _selectedPlace,
			Text _infoPlace
	) {
		selectedPlace = _selectedPlace;
		infoPlace = _infoPlace;
	}


	@Override
	public void block() {
		// Place data should not be blocked
		// It is just a text and not an interactive element
	}

	@Override
	public void unblock() {
		// If it never blocks, it will not need to be unblocked
	}

	@Override
	public void clear() {
		Platform.runLater(() -> {
			selectedPlace.setText("UNKNOWN");
			infoPlace.setText("UNKNOWN");
		});
	}

	@Override
	public void update(PlaceInfo newPlaceInfo) {
		Platform.runLater(() -> {
			selectedPlace.setText(newPlaceInfo.getPlaceName());
			if (newPlaceInfo.getPlaceInfo().equals("")) {
				infoPlace.setText("No information given");
			}
			else {
				infoPlace.setText(newPlaceInfo.getPlaceInfo());
			}
		});
	}
}
