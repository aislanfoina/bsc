package br.usp.lme.smartv.bttest;

import java.util.Date;

public class BTPresenceListComponent {
	protected String btAddress = "";
	protected Date time;
	protected int flag = 0;

	BTPresenceListComponent() {
		time = new Date(System.currentTimeMillis());
	}

	BTPresenceListComponent(String btAddress) {
		this.btAddress = btAddress;
		this.time = new Date(System.currentTimeMillis());
	}

	boolean compareBtAddress(BTPresenceListComponent btComponent) {
		if(btAddress.equals(btComponent.getBtAddress())) {
			return true;
		}
		return false;
	}

	/**
	 * @return Returns the btAddress.
	 */
	public String getBtAddress() {
		return btAddress;
	}

	/**
	 * @param btAddress The btAddress to set.
	 */
	public void setBtAddress(String btAddress) {
		this.btAddress = btAddress;
	}

	/**
	 * @return Returns the flag.
	 */
	public int getFlag() {
		return flag;
	}

	/**
	 * @param flag The flag to set.
	 */
	public void setFlag(int flag) {
		this.flag = flag;
	}

	/**
	 * @return Returns the time.
	 */
	public Date getTime() {
		return time;
	}

	/**
	 * @param time The time to set.
	 */
	public void setTime(Date time) {
		this.time = time;
	}
	
	
}
