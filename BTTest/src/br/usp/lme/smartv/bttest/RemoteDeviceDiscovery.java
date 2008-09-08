package br.usp.lme.smartv.bttest;

/**
 *  BlueCove - Java library for Bluetooth
 *  Copyright (C) 2006-2008 Vlad Skarzhevskyy
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  @version $Id$
 */

import java.io.IOException;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Vector;

import javax.bluetooth.*;

/**
 * @author vlads
 *
 * Minimal DeviceDiscovery example for javadoc.
 */
public class RemoteDeviceDiscovery {

    public static final Vector/*<RemoteDevice>*/ devicesDiscovered = new Vector();
    public static final BTPresenceList devicesPresent = new BTPresenceList();

    public static void main(String[] args) throws IOException, InterruptedException {

        final Object inquiryCompletedEvent = new Object();

        int maxLoop = 0;
        
        while (maxLoop < 10) {
        
        maxLoop++;
        	
        devicesDiscovered.clear();

        DiscoveryListener listener = new DiscoveryListener() {

            public void deviceDiscovered(RemoteDevice btDevice, DeviceClass cod) {
                try {
//                System.out.println("Device " + btDevice.getBluetoothAddress() + " found");
                	devicesDiscovered.addElement(btDevice);
                	if(!devicesPresent.isPresent(btDevice.getBluetoothAddress())) {
                		System.out.println("New Device Arrived! BTAddress = " + btDevice.getBluetoothAddress()+ "("+btDevice.getFriendlyName(false)+")");
                		devicesPresent.add(new BTPresenceListComponent(btDevice.getBluetoothAddress()));
                	}
                } catch (IOException cantGetDeviceName) {
                }
            }

            public void inquiryCompleted(int discType) {
                System.out.println("Device Inquiry completed!");
                synchronized(inquiryCompletedEvent){
                    inquiryCompletedEvent.notifyAll();
                }
            }

            public void serviceSearchCompleted(int transID, int respCode) {
            }

            public void servicesDiscovered(int transID, ServiceRecord[] servRecord) {
            }
        };
        
        synchronized(inquiryCompletedEvent) {
            boolean started = LocalDevice.getLocalDevice().getDiscoveryAgent().startInquiry(DiscoveryAgent.GIAC, listener);
            if (started) {
//                System.out.println("wait for device inquiry to complete...");
                inquiryCompletedEvent.wait();
                
    			try {

    			for (Iterator i = devicesPresent.iterator(); i.hasNext() && !devicesPresent.isEmpty(); ) {
        			BTPresenceListComponent btComponentTemp = (BTPresenceListComponent)i.next();
        			boolean isPresent = false;
            		for (Iterator j = devicesDiscovered.iterator(); j.hasNext(); ) {
            			RemoteDevice btRemoteDeviceTemp = (RemoteDevice) j.next();
            			if(btComponentTemp.getBtAddress().equals(btRemoteDeviceTemp.getBluetoothAddress())) {
            				isPresent = true;
            			}
            		}
            		if (!isPresent) {
            			System.out.println("Device left! BTAddress = " + btComponentTemp.getBtAddress());
//            			devicesPresent.remove(btComponentTemp.getBtAddress());
            			devicesPresent.remove(btComponentTemp); // FIXME
            		}
            	}

            	} catch (Exception e1) {
				System.err.print(e1.getMessage());
        		}
       
                
 //               System.out.println(devicesDiscovered.size() +  " device(s) found");
            }
        }
        
        }
    }
}
