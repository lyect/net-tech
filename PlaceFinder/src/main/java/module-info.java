module com.lyect.placefinder {
	requires javafx.controls;
	requires javafx.fxml;
	requires java.net.http;
	requires com.fasterxml.jackson.databind;

	opens com.lyect.placefinder to javafx.fxml;
	exports com.lyect.placefinder;
	exports com.lyect.placefinder.controllers;
	opens com.lyect.placefinder.controllers to javafx.fxml;
	exports com.lyect.placefinder.controllers.activitycontrollers;
	opens com.lyect.placefinder.controllers.activitycontrollers to javafx.fxml;
	exports com.lyect.placefinder.controllers.choicecontrollers;
	opens com.lyect.placefinder.controllers.choicecontrollers to javafx.fxml;
	exports com.lyect.placefinder.controllers.infocontrollers;
	opens com.lyect.placefinder.controllers.infocontrollers to javafx.fxml;
}