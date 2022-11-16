package com.lyect.placefinder.controllers;

import com.lyect.placefinder.containers.*;
import com.lyect.placefinder.controllers.choicecontrollers.ChoiceController;
import com.lyect.placefinder.controllers.activitycontrollers.FindButtonController;
import com.lyect.placefinder.controllers.choicecontrollers.LocationChoiceController;
import com.lyect.placefinder.controllers.choicecontrollers.PlaceChoiceController;
import com.lyect.placefinder.controllers.infocontrollers.InfoController;
import com.lyect.placefinder.controllers.infocontrollers.PlaceInfoController;
import com.lyect.placefinder.controllers.infocontrollers.LocationInfoController;
import com.lyect.placefinder.requester.Requester;
import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.text.Text;
import javafx.scene.text.TextFlow;

import java.util.List;
import java.util.concurrent.CompletableFuture;

public class MainController {

	@FXML
	private TextField findField;
	@FXML
	private Button findButton;

	@FXML
	private ChoiceBox<Location> locationChoice;
	@FXML
	private ChoiceBox<Place> placeChoice;

	@FXML
	private Label selectedLocation;
	@FXML
	private Label temperatureLocation;
	@FXML
	private Label cloudsLocation;

	@FXML
	private Label selectedPlace;
	@FXML
	private Text infoPlace;

	private Controller findButtonController;
	private ChoiceController<Location> locationChoiceController;
	private ChoiceController<Place> placeChoiceController;

	InfoController<LocationInfo> locationInfoController;
	InfoController<PlaceInfo> placeInfoController;

	@FXML
	private void initialize() {

		initializeControllers();

		// Set actions for interactive elements
		setFindButtonAction();
		setLocationChoiceAction();
		setPlaceChoiceAction();

		// Clear location and place data
		clearData();

		// Block choice boxes because there is no locations or
		//	places have been found yet
		locationChoiceController.block();
		placeChoiceController.block();
	}

	private void initializeControllers() {
		// Initialize activity controllers
		findButtonController = new FindButtonController(findButton);
		locationChoiceController = new LocationChoiceController(locationChoice);
		placeChoiceController = new PlaceChoiceController(placeChoice);

		// Initialize info controllers
		locationInfoController = new LocationInfoController(
				selectedLocation,
				temperatureLocation,
				cloudsLocation
		);
		placeInfoController = new PlaceInfoController(
			selectedPlace,
			infoPlace
		);
	}

	private void setFindButtonAction() {
		findButton.setOnAction(event -> {
			// Get written value from text field
			String stringLocation = findField.getText();

			// Block find button until location is found
			findButtonController.block();

			locationChoiceController.block();
			placeChoiceController.block();

			// When request in process, location info and place info
			//	have to be cleared (set to its default values)
			clearData();

			// Make async HTTP request. It returns list filled with locations with similar names
			CompletableFuture<List<Location>> foundLocations =
					Requester.makeFindLocationRequest(stringLocation);
			// Update info
			foundLocations.thenAccept(data -> {
				// Unblock find button
				findButtonController.unblock();
				// Update value list in location choice box
				locationChoiceController.update(data);
				// Unblock choice box with location selection
				locationChoiceController.unblock();
			});
		});
	}

	private void setLocationChoiceAction() {
		locationChoice.setOnAction(event -> {
			// Get selected value from choice box
			Location selectedLocation = locationChoice.getValue();

			// Handler triggers when choice box is cleared
			if (selectedLocation == null) {
				return;
			}

			// Block choice boxes until corresponding requests are done
			locationChoiceController.block();
			placeChoiceController.block();

			// Make async HTTP request. It returns data about selected location
			CompletableFuture<LocationInfo> locationInfo =
					Requester.makeLocationInfoRequest(selectedLocation);
			// Update info
			locationInfo.thenAccept(info -> {
				// Update location data
				locationInfoController.update(info);
				// Unblock choice box with location selection
				locationChoiceController.unblock();
			});

			// Make async HTTP request. It returns places within chosen location
			CompletableFuture<List<Place>> foundPlaces =
					Requester.makeFindPlacesRequest(selectedLocation);
			// Update info
			foundPlaces.thenAccept(data -> {
				// Update value list in place choice box
				placeChoiceController.update(data);
				// Unblock choice box with place selection
				placeChoiceController.unblock();
			});
		});
	}

	private void setPlaceChoiceAction() {
		placeChoice.setOnAction(event -> {
			// Get selected value from choice box
			Place selectedPlace = placeChoice.getValue();

			// Handler triggers when choice box is cleared
			if (selectedPlace == null) {
				return;
			}

			// Block choice box until corresponding request is done
			placeChoiceController.block();

			// Make async HTTP request. It returns data about selected place
			CompletableFuture<PlaceInfo> placeInfo = Requester.makePlaceInfoRequest(selectedPlace);
			// Update info
			placeInfo.thenAccept(info -> {
				// Update place data
				placeInfoController.update(info);
				// Unblock choice box with place selection
				placeChoiceController.unblock();
			});
		});
	}

	// Clears info about locations and places
	// Also blocks choice boxes until corresponding requests are done
	private void clearData() {
		// Clear value lists in choice boxes
		locationChoiceController.clear();
		placeChoiceController.clear();
		// Set location and place data to its default values
		locationInfoController.clear();
		placeInfoController.clear();
	}
}
