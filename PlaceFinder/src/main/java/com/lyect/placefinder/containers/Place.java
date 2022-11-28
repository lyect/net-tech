package com.lyect.placefinder.containers;

public class Place {

	private final String name;
	private final String xid;

	public Place() {
		name = "";
		xid = "";
	}

	public Place(String _name, String _xid) {
		name = _name;
		xid = _xid;
	}

	public String getName() {
		return name;
	}

	public String getXid() {
		return xid;
	}

	@Override
	public String toString() {
		return getName();
	}
}
