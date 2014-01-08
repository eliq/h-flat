#ifndef FLAT_NAMESPACE_H_
#define FLAT_NAMESPACE_H_

#include "metadata_info.h"
#include "database.pb.h"

#include "kinetic/status.h"
/*
 * Let`s just pretend not to see the use of KineticStatus in this class.
 */

class NamespaceStatus final : public kinetic::KineticStatus
{
public:
      static NamespaceStatus makeInvalid() {
    	  NamespaceStatus ret("Invalid value");
          ret.invalid_value_ = true;
          return ret;
      }

      NamespaceStatus(std::string error_message)
          : kinetic::KineticStatus(error_message), invalid_value_(false){
      }
      NamespaceStatus(kinetic::KineticStatus kinetic)
      	  : kinetic::KineticStatus(kinetic), invalid_value_(false){
      }
      NamespaceStatus(kinetic::Status status)
      	  : kinetic::KineticStatus(status), invalid_value_(false){
      }


      bool notValid() const {
          return invalid_value_;
      }

  private:
      bool invalid_value_;
};



enum class PutModeType {
	POSIX, 		// last writer wins, just overwrite without version checks
	ATOMIC 		// only write if remote data version has not been updated since read in by this client
};


/* Interface for the underlying key-value store. It's deliberately high-level: exporting multiple put / get methods enables optimized
 * implementation depending on the type of key being processed while keeping such details out of the file system itself. */
class FlatNamespace{
public:
	/* File Metadata */
	virtual NamespaceStatus getMD( 	  MetadataInfo *mdi)  = 0;
	virtual NamespaceStatus putMD( 	  MetadataInfo *mdi)  = 0;
	virtual NamespaceStatus deleteMD( MetadataInfo *mdi ) = 0;

	/* File Data */
	virtual NamespaceStatus get(	MetadataInfo *mdi, unsigned int blocknumber, std::string *value) = 0;
	virtual NamespaceStatus put(	MetadataInfo *mdi, unsigned int blocknumber, const std::string &value, const PutModeType type = PutModeType::POSIX) = 0;
	virtual NamespaceStatus free(   MetadataInfo *mdi, unsigned int blocknumber) = 0;

	/* Database Handling */
	virtual NamespaceStatus putDBEntry(		std::int64_t  versionDB, const posixok::db_entry &entry) = 0;
	virtual NamespaceStatus getDBEntry( 	std::int64_t  versionDB, posixok::db_entry &entry) = 0;
	virtual NamespaceStatus getDBVersion( 	std::int64_t &versionDB) = 0;

public:
	virtual ~FlatNamespace(){};
};


#endif /* FLAT_NAMESPACE_H_ */