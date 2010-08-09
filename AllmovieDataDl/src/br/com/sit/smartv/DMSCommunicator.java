/*
 * Created on 15/06/2004
 */
package br.com.sit.smartv;

import java.io.*;
import java.net.*;

/**
 * @author Aislan Gomide Foina
 * 
 * Responsavel pela interface grafica do sistema
 */
public class DMSCommunicator {

	static int retry = 0;
	
	public DMSCommunicator() {
	}

	/**
	 * Construtor
	 * @param host em q sera conectado
	 */
	public DMSCommunicator(String host) {
		httpConnect(host, "");		
	}
	/**
	 * Contrutor que recebe o host e os dados
	 * 
	 * @param host Host a ser conectado
	 * @param data Parametros a serem passados
	 */
	public DMSCommunicator(String host, String data) {
		httpConnect(host, data);		
	}
	/**
	 * Construtor que conecta no host e tem os campos um a um como parametro
	 * 
	 * @param host Host a conectar
	 * @param mapurl URL do mapa
	 * @param lat Latitude
	 * @param lon Longitude
	 * @param latmin Latitude minima
	 * @param latmax Latitude maxima
	 * @param lonmin Longitude minima
	 * @param lonmax Longitude maxima
	 */
	public DMSCommunicator(String host, String mapurl, double lat, double lon, double latmin, double latmax, double lonmin, double lonmax) {
		String data;
		
		data = "action=Enviar&mapaurl="+mapurl+"&latitude="+lat+"&longitude="+lon+"&minlat="+latmin+"&maxlat="+latmax+"&minlon="+lonmin+"&maxlon="+lonmax;
		System.out.println("\n"+host+"?"+data);
		httpConnect(host,data);		
	}

	/**
	 * Conecta no host e envia os dados
	 * 
	 * @param host Host a se conectar
	 * @param data Dados a serem enviados para o host
	 */
	public void httpConnect (String host, String data) {
		URL url;
//		DataOutputStream httpout;		
		try	{
			url = new URL(host);
			URLConnection conn = url.openConnection();
			conn.setDoOutput(true);
			OutputStreamWriter wr = new OutputStreamWriter(conn.getOutputStream());
			String dataform = data;
			wr.write(dataform);
			wr.flush();
			BufferedReader rd = new BufferedReader(new InputStreamReader(conn.getInputStream()));
//			String line;
			while ((/*line = */rd.readLine()) != null) {
			}
			wr.close();
			rd.close();
			
		} catch (MalformedURLException me) {
			System.err.println("DMSCommunicator: MalformedURLException: " + me);
		} catch (IOException ioe) {
			System.err.println("DMSCommunicator: IOException: " + ioe.getMessage());
		}	
	}
	
	public static void httpSendXmlNoAswer(String host) {
		URL url;
		HttpURLConnection http;
		BufferedReader httpin;
		String output = new String();
		
		try	{
			url = new URL (host);
			http = (HttpURLConnection) url.openConnection();
			http.setDoInput (true);
			http.setDoOutput (true);
			http.setUseCaches (false);
			http.connect();
		} catch (MalformedURLException me) {
			System.err.println("DMSCommunicator: MalformedURLException: " + me);
		} catch (IOException ioe) {
			System.err.println("DMSCommunicator: IOException: " + ioe.getMessage());
		}
	}	
	
	public static String httpSendXml(String host) {
		URL url;
		HttpURLConnection http;
		BufferedReader httpin;
		String output = new String();
		
		try	{
			url = new URL (host);
			http = (HttpURLConnection) url.openConnection();
			http.setDoInput (true);
			http.setDoOutput (true);
			http.setUseCaches (false);
			http.connect();
			httpin = new BufferedReader(new InputStreamReader(http.getInputStream()));
			String line;
			while ((line = httpin.readLine()) != null) {
				output = output.concat(line+"\n");
			}
			httpin.close();
			
		} catch (MalformedURLException me) {
			System.err.println("DMSCommunicator: MalformedURLException: " + me);
		} catch (IOException ioe) {
			System.err.println("DMSCommunicator: IOException: " + ioe.getMessage());
		}

		return output;
	}

	public static String httpSendXml(String host, String xml) {
		URL url;
		HttpURLConnection http;
		OutputStreamWriter httpout;
		BufferedReader httpin;
		String output = new String();
		
		try	{
			url = new URL (host);
			http = (HttpURLConnection) url.openConnection();
			http.setDoInput (true);
			http.setDoOutput (true);
			http.setUseCaches (false);
			http.setRequestMethod ("POST");
			http.setRequestProperty ("Content-Type", "text/xml");
			http.connect();
			httpout = new OutputStreamWriter (http.getOutputStream ());
			httpout.write (xml);
			httpout.flush ();

			httpin = new BufferedReader(new InputStreamReader(http.getInputStream()));
			String line;
			while ((line = httpin.readLine()) != null) {
				output = output.concat(line+"\n");
			}
			httpin.close();
			httpout.close ();
			retry = 0;
			
		} catch (MalformedURLException me) {
			System.err.println("DMSCommunicator: MalformedURLException: " + me);
		} catch (IOException ioe) {
			retry++;
			if (retry < 3) {
				System.err.println("DMSCommunicator: IOException: " + ioe.getMessage()+ " retry " + retry + "...");
				httpSendXml(host,"");
			}
			else {
				System.err.println("DMSCommunicator: IOException: " + ioe.getMessage());
				retry = 0;

			}
			
		}

		return output;
	}
}
