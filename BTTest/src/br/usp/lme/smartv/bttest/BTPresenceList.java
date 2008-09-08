package br.usp.lme.smartv.bttest;

import java.util.ArrayList;
import java.util.Iterator;

public class BTPresenceList extends ArrayList{

	private static final long serialVersionUID = 1L;
	
	public void resetFilter() {
		for (Iterator i = this.iterator(); i.hasNext(); ) {
			this.remove(i.next());
		}
	}
	
	public void remove(String BTAddress) {
		for (Iterator i = this.iterator(); i.hasNext() && !this.isEmpty(); ) {
			BTPresenceListComponent btComponentTemp = (BTPresenceListComponent)i.next();
			if(btComponentTemp.getBtAddress().equals(BTAddress))
				this.remove(btComponentTemp);
		}
	}

	public boolean isPresent(String BTAddress) {
		for (Iterator i = this.iterator(); i.hasNext(); ) {
			BTPresenceListComponent btComponentTemp = (BTPresenceListComponent)i.next();
			if(btComponentTemp.getBtAddress().equals(BTAddress))
				return true;
		}
		return false;
	}
	
}
