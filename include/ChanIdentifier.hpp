#ifndef __CHANIDENTIFIER_HPP
#define __CHANIDENTIFIER_HPP

#include <sstream>
#include <string>
#include <map>
#include <string>
#include <iomanip>
#include <iostream>

/**
 * \brief Channel identification

 * All parameters needed to uniquely specify the detector connected to a
 * specific channel are set here.  A vector of identifiers for all channels is
 * created on initialization in InitID().  Each event that is created in
 * ScanList() in PixieStd.cpp has its channel identifier attached to it in
 * the variable chanID inside RawEvent
 *
 * Identifier is a class that will contain basic channel information that
 * will not change including the damm spectrum number where the raw energies
 * will be plotted, the detector type and subtype, and the detector's physical
 * location (strip number, detector location, ...)
 */
class Identifier
{
public:
    void SetDammID(int a){dammID = a;}   /**< Set the dammid */
    void SetType(const std::string &a){type = a;}     /**< Set the detector type */
    void SetSubtype(const std::string &a){subtype = a;}  /**< Set the detector subtype */
    void SetLocation(int a){location = a;} /**< Set the detector location */
    
    int GetDammID() const {return dammID;}   /**< Get the dammid */
    const std::string& GetType() const {return type;}     /**< Get the detector type */
    const std::string& GetSubtype() const {return subtype;}  /**< Get the detector subtype */
    int GetLocation() const {return location;} /**< Get the detector location */
    
    void AddTag(const std::string &s, int n){tag[s] = n;} /**< Insert a tag */
    bool HasTag(const std::string &s) const {return (tag.count(s) > 0);} /**< True if the tag s has been inserted */
    int GetTag(const std::string &s) const; 

    Identifier();
    void Zero();
    static void PrintHeaders(void);
    void Print(void) const;
    
    bool operator==(const Identifier &x) const {
		return (type == x.type && subtype == x.subtype && location == x.location);
    } /**< Compare this identifier with another */
    bool operator!=(const Identifier &x) const {
		return !operator==(x);
    }

    std::string GetPlaceName() const {
        std::stringstream ss;
        ss << GetType() << "_" << GetSubtype() << "_" << GetLocation();
        return ss.str();
    }
    
    void Dump(){
    	std::cout << " type: " << type << "\tsubtype: " << subtype << "\tlocation: " << location << std::endl;
    	for(std::map<std::string, int>::iterator iter = tag.begin(); iter != tag.end(); iter++){
    		std::cout << "  tag: " << iter->first << "\tid: " << iter->second << std::endl;
    	}
    }

private:
    std::string type; /**< Specifies the detector type */
    std::string subtype; /**< Specifies the detector sub type */
    int dammID; /**< Damm spectrum number for plotting calibrated energies */
    int location; /**< Specifies the real world location of the channel. For the DSSD this variable is the strip number */
    std::map<std::string, int> tag; /**< A list of tags associated with the identifer */ 
};

#endif
