package br.com.sit.smartv;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;

public class Movie {
	int id;
	String origName;
	String year;
	
	String Category;
	String Genre1;
	String Genre2;
	String Genre3;

	int maxAge_tol = 0;
	
	public void print() {
		System.out.println("ID "+id);
		System.out.println("Title "+origName);
		System.out.println("Year "+year);
		System.out.println("Category "+Category);
		System.out.println("Genre1 "+Genre1);
		System.out.println("Genre2 "+Genre2);
		System.out.println("Genre3 "+Genre3);
		System.out.println("");
		System.out.println("");
	}
	
	public void save(Connection conn) throws SQLException {
		String name = origName.replace('\'',' ');
		String Category = this.Category.replace('\'',' ');
		String Genre1 = this.Genre1.replace('\'',' ');
		String Genre2 = this.Genre2.replace('\'',' ');
		String Genre3 = this.Genre3.replace('\'',' ');
		
		Statement stmt2 = conn.createStatement();
		stmt2.executeUpdate("insert into movies_allmovie values ("+id+","+year+",'"+name+"','"+Category+"','"+Genre1+"','"+Genre2+"','"+Genre3+"');");
	}

	public int search (String type, Connection conn) {
		boolean found = true;
		int returnVal = 0;
		
		String Category = "";
		String Genre1 = "";
		String Genre2 = "";
		String Genre3 = "";

		String name = origName;
		
		name = name.replace("%","");
		name = name.replace(":","");
		name = name.replace("?","");
		name = name.replace("/","");
		name = name.replace(".","");
		name = name.replace("-"," ");

		String nameUrl = name.replace(" ","+");

		String output = DMSCommunicator.httpSendXml("http://www.allmovie.com/search/"+type+"/"+nameUrl+"/results");
		
		if(!output.contains("Sorry, no search results found.")) {
			try {
				int indexStart = output.indexOf("td class=\"cell\" style=\"width: 284px;\"");
				String searchResult = output.substring(indexStart);
				int indexEnd = searchResult.indexOf("</tr>");
				searchResult = searchResult.substring(0,indexEnd);

				String movieUrl = searchResult.substring(searchResult.indexOf("href=\""));
				movieUrl = movieUrl.substring(movieUrl.indexOf("http"),movieUrl.indexOf("\">"));
				
				String movieYear = searchResult.substring(searchResult.indexOf("<td class=\"cell\" style=\"width: 70px;\">"));
				movieYear = movieYear.substring(movieYear.indexOf(">")+1,movieYear.indexOf("</td>"));
				
				if(checkAge(year, movieYear)) {
					output = DMSCommunicator.httpSendXml(movieUrl);
					int indexCategory = output.indexOf(">Category<");
					if (indexCategory != 0) {
						String category = output.substring(indexCategory);
						category = category.substring(category.indexOf("<a"),category.indexOf("</a"));
						category = category.substring(category.indexOf(">")+1);
						Category = category;
					}
					int indexGenres = output.indexOf("Genres");
					if (indexGenres > 0) {
						String genres = output.substring(indexGenres);
						genres = genres.substring(genres.indexOf("ul>")+3,genres.indexOf("</ul>"));
						String genre1 = genres.substring(genres.indexOf("<li>")+4, genres.indexOf("</li>"));
						if(genre1.contains("<a href")) {
							genre1 = genre1.substring(genre1.indexOf(">")+1,genre1.indexOf("</a>"));
						}
						Genre1 = genre1;
						
						String genre2 = genres.substring(genres.indexOf("</li>")+5);
						String genre3 = genre2.substring(genre2.indexOf("</li>")+5);

						if(genre2.contains("<li")) {
							genre2 = genre2.substring(genre2.indexOf("<li>")+4, genre2.indexOf("</li>"));
							if(genre2.contains("<a href")) {
								genre2 = genre2.substring(genre2.indexOf(">")+1,genre2.indexOf("</a>"));
							}
							Genre2 = genre2;
							
							if(genre3.contains("<li")) {
								genre3 = genre3.substring(genre3.indexOf("<li>")+4, genre3.indexOf("</li>"));
								if(genre3.contains("<a href")) {
									genre3 = genre3.substring(genre3.indexOf(">")+1,genre3.indexOf("</a>"));
								}
								Genre3 = genre3;
							}								
						}
//						okcnt++;
						returnVal = 0;
					}
					else {
						System.out.println("ID "+id+" genre not found!");
//						ngenre++;
						returnVal = 1;
						found = false;
					}
				}
				else {
					System.out.println("ID "+id+" year not match: "+year+" != "+movieYear);
//					nyear++;
					returnVal = 2;
					found = false;
				}
				
			} catch (Exception e) {
				// TODO: handle exception
				System.err.println("Exception: ID "+id+": " + e);
//				errcnt++;
				returnVal = 3;
				found = false;
			}
		}
		else {
			System.out.println("ID "+id+" not found: "+name);
//			nfcnt++;
			returnVal = 4;
			found = false;
		}
		
//		this.setId(id);
//		this.setYear(year);
//		this.setOrigName(origName);
		this.setCategory(Category);
		this.setGenre1(Genre1);
		this.setGenre2(Genre2);
		this.setGenre3(Genre3);
	
		return returnVal;
	}
	
	public boolean checkAge (String yearRef, String yearCmp) {
		try {
			int intYR = Integer.parseInt(yearRef);
			int intYC = Integer.parseInt(yearCmp);

			int diff =  intYR - intYC;
		
			if(Math.abs(diff) > maxAge_tol)		
				return false;
			else
				return true;
		} catch (Exception e) {
			return false;
		}
	}
	
	/**
	 * @return Returns the category.
	 */
	public String getCategory() {
		return Category;
	}

	/**
	 * @param category The category to set.
	 */
	public void setCategory(String category) {
		Category = category;
	}

	/**
	 * @return Returns the genre1.
	 */
	public String getGenre1() {
		return Genre1;
	}

	/**
	 * @param genre1 The genre1 to set.
	 */
	public void setGenre1(String genre1) {
		Genre1 = genre1;
	}

	/**
	 * @return Returns the genre2.
	 */
	public String getGenre2() {
		return Genre2;
	}

	/**
	 * @param genre2 The genre2 to set.
	 */
	public void setGenre2(String genre2) {
		Genre2 = genre2;
	}

	/**
	 * @return Returns the genre3.
	 */
	public String getGenre3() {
		return Genre3;
	}

	/**
	 * @param genre3 The genre3 to set.
	 */
	public void setGenre3(String genre3) {
		Genre3 = genre3;
	}

	/**
	 * @return Returns the id.
	 */
	public int getId() {
		return id;
	}

	/**
	 * @param id The id to set.
	 */
	public void setId(int id) {
		this.id = id;
	}

	/**
	 * @return Returns the origName.
	 */
	public String getOrigName() {
		return origName;
	}

	/**
	 * @param origName The origName to set.
	 */
	public void setOrigName(String origName) {
		this.origName = origName;
	}

	/**
	 * @return Returns the year.
	 */
	public String getYear() {
		return year;
	}

	/**
	 * @param year The year to set.
	 */
	public void setYear(String year) {
		this.year = year;
	}

	/**
	 * @return Returns the maxAge_tol.
	 */
	public int getMaxAge_tol() {
		return maxAge_tol;
	}

	/**
	 * @param maxAge_tol The maxAge_tol to set.
	 */
	public void setMaxAge_tol(int maxAge_tol) {
		this.maxAge_tol = maxAge_tol;
	}
	
}
