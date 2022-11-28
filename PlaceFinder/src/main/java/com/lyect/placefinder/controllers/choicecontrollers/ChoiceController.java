package com.lyect.placefinder.controllers.choicecontrollers;

import com.lyect.placefinder.controllers.Controller;

import java.util.List;

public interface ChoiceController<T> extends Controller {
	@Override
	void block();

	@Override
	void unblock();

	@Override
	void clear();

	void update(List<T> newValues);
}
