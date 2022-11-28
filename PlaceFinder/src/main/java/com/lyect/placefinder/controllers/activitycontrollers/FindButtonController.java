package com.lyect.placefinder.controllers.activitycontrollers;

import com.lyect.placefinder.controllers.Controller;
import javafx.scene.control.Button;

public class FindButtonController implements Controller {
	private final Button findButton;
	private final Object blockingSynchronization = new Object();
	public FindButtonController(Button _findButton) {
		findButton = _findButton;
	}

	@Override
	public void block() {
		synchronized (blockingSynchronization) {
			findButton.setDisable(true);
		}
	}

	@Override
	public void unblock() {
		synchronized (blockingSynchronization) {
			findButton.setDisable(false);
		}
	}

	@Override
	public void clear() {
		// Button does not need to be cleared
	}
}
