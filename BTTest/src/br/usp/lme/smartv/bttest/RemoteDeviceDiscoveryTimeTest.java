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
import java.util.Date;
import java.util.Iterator;
import java.util.Vector;

import javax.bluetooth.*;

/**
 * @author vlads
 *
 * Minimal DeviceDiscovery example for javadoc.
 */
public class RemoteDeviceDiscoveryTimeTest {

    public static final Vector/*<RemoteDevice>*/ devicesDiscovered = new Vector();
    public static final BTPresenceList devicesPresent = new BTPresenceList();
    public static Date dtStart; 

    public static void main(String[] args) throws IOException, InterruptedException {

        final Object inquiryCompletedEvent = new Object();

        int maxLoop = 0;
        
        while (maxLoop < 20) {


        	
        maxLoop++;
        	
        devicesDiscovered.clear();


    	DiscoveryListener listener = new DiscoveryListener() {

            public void deviceDiscovered(RemoteDevice btDevice, DeviceClass cod) {
                try {
//                System.out.println("Device " + btDevice.getBluetoothAddress() + " found");
                	Date dtNow = new Date(System.currentTimeMillis());
                	Date deltaDt = new Date(dtNow.getTime() - dtStart.getTime());
                	System.out.println(dtNow.getTime() - dtStart.getTime() +"   BTAddress = " + btDevice.getBluetoothAddress()+ "("+btDevice.getFriendlyName(false)+")");
                } catch (IOException cantGetDeviceName) {
                }
            }

            public void inquiryCompleted(int discType) {
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
            	dtStart = new Date(System.currentTimeMillis());
                System.out.println("Starting device inquiry...");
                inquiryCompletedEvent.wait();
                
                
                System.out.println("Loop    "+maxLoop+" - Device Inquiry completed!\n");
            }
        }
        
        }
    }
}
