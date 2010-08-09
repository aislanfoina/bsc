package br.com.sit.smartv;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class AllmovieConnector {

	public static void main(String[] args) {

		int totalcnt = 0;
		int okcnt = 0;
		int errcnt = 0;
		int nfcnt = 0;
		int nyear = 0;
		int ngenre = 0;

		int cnt1 = 0;
		int cnt2 = 0;
		int cnt3 = 0;
		int cnt4 = 0;
		int cnt5 = 0;
		int cnt6 = 0;

		try {
			String host = "localhost";
			String port = "3306";
			String db = "sandbox";
			String user = "root";
			String pass = "";

			Class.forName("com.mysql.jdbc.Driver");
			Connection conn = DriverManager.getConnection("jdbc:mysql://"
					+ host + ":" + port + "/" + db, user, pass);

			System.out.println("Connected SQL Server!");
			// create our java jdbc statement
			Statement stmt = conn.createStatement();
			ResultSet rs;

			stmt.executeUpdate("delete from movies_allmovie");

			rs = stmt.executeQuery("select id, title, year from movies_clean");

			while (rs.next()) {

				totalcnt++;

				int id = rs.getInt("id");
				String name = rs.getString("title");
				String year = rs.getString("year");

				Movie movie = new Movie();

				movie.setId(id);
				movie.setOrigName(name);
				movie.setYear(year);

				int ret = movie.search("all", conn);

				boolean ok = false;

				if (ret == 0) {
					cnt1++;
					okcnt++;
					ok = true;
				} else {
					ret = movie.search("work", conn);
					if (ret == 0) {
						cnt2++;
						okcnt++;
						ok = true;
					} else {
						ret = movie.search("dvd", conn);
						if (ret == 0) {
							cnt3++;
							okcnt++;
							ok = true;
						} else {
							boolean workYear0 = false;
							movie.setMaxAge_tol(1);
							ret = movie.search("work", conn);
							if (ret == 0) {
								cnt4++;
								okcnt++;
								ok = true;
							} else if (ret == 2) {
								if (movie.getFoundYear().equals("")) {
									workYear0 = true;
								}
							} else {
								boolean dvdYear0 = false;
								ret = movie.search("dvd", conn);
								if (ret == 0) {
									cnt5++;
									okcnt++;
									ok = true;
								} else if (ret == 2) {
									if (movie.getFoundYear().equals("")) {
										dvdYear0 = true;
									}
								}
								if (dvdYear0 || workYear0) {
									ret = movie.search("dvd", conn, true);
									if (ret == 0 && movie.getFoundYear().equals("")) {
										movie.setYear("0");
										cnt6++;
										okcnt++;
										ok = true;
									} else {
										ret = movie.search("work", conn, true);
										if (ret == 0 && movie.getFoundYear().equals("")) {
											movie.setYear("0");
											cnt6++;
											okcnt++;
											ok = true;
										}
									}
								}
							}
						}
					}
				}
				if (!ok) {
					switch (ret) {
					case 1:
						ngenre++;
						break;
					case 2:
						nyear++;
						break;
					case 3:
						errcnt++;
						break;
					case 4:
						nfcnt++;
						break;
					default:
						break;
					}
				}
				movie.print();
				movie.save(conn);

				if (totalcnt % 25 == 0) {
					System.out.println("Total:          " + totalcnt);
					System.out.println();
					System.out.println("Ok:             " + okcnt);
					System.out.println("Errors:         " + errcnt);
					System.out.println("Movie NF:       " + nfcnt);
					System.out.println("Year Mistmatch: " + nyear);
					System.out.println("Genre NF:       " + ngenre);
					System.out.println();
					System.out.println("Cnt1:       " + cnt1);
					System.out.println("Cnt2:       " + cnt2);
					System.out.println("Cnt3:       " + cnt3);
					System.out.println("Cnt4:       " + cnt4);
					System.out.println("Cnt5:       " + cnt5);
				}
			}
			conn.close();

		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		System.out.println("==============================");
		System.out.println("Total:          " + totalcnt);
		System.out.println();
		System.out.println("Ok:             " + okcnt);
		System.out.println("Errors:         " + errcnt);
		System.out.println("Movie NF:       " + nfcnt);
		System.out.println("Year Mistmatch: " + nyear);
		System.out.println("Genre NF:       " + ngenre);
		System.out.println();
		System.out.println("Cnt1:       " + cnt1);
		System.out.println("Cnt2:       " + cnt2);
		System.out.println("Cnt3:       " + cnt3);
		System.out.println("Cnt4:       " + cnt4);
		System.out.println("Cnt5:       " + cnt5);
		System.out.println("Cnt6:       " + cnt6);
		System.out.println("==============================");
	}
}