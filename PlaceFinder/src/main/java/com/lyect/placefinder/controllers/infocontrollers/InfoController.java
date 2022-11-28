package com.lyect.placefinder.controllers.infocontrollers;

import com.lyect.placefinder.controllers.Controller;

public interface InfoController<T> extends Controller {
	@Override
	void block();

	@Override
	void unblock();

	@Override
	void clear();

	void update(T newData);
}
