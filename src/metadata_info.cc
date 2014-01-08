#include "metadata_info.h"
#include <chrono>
#include <sys/stat.h>


posixok::Metadata * MetadataInfo::pbuf(){
	return &md;
}

const std::string & MetadataInfo::getSystemPath(){
	return systemPath;
}

const std::string & MetadataInfo::getCurrentVersion(){
	return currentVersion;
}

void MetadataInfo::setSystemPath(const std::string &path){
	systemPath=path;
}

void MetadataInfo::setCurrentVersion(const std::string &version){
	currentVersion=version;
}

void MetadataInfo::updateACMtime(){
	std::time_t now;
	std::time(&now);
	md.set_atime(now);
	md.set_mtime(now);
	md.set_ctime(now);
}

void MetadataInfo::updateACtime(){
	std::time_t now;
	std::time(&now);
	md.set_atime(now);
	md.set_ctime(now);
}


void MetadataInfo::trackDataVersion(int blockNumber, const std::string &keyVersion)
{
	dataVersion[blockNumber] = keyVersion;
}

std::string MetadataInfo::getDataVersion(int blockNumber)
{
	return dataVersion[blockNumber];
}


bool MetadataInfo::computePathPermissionChildren()
{
	/* Execute permission for user / group / other */
	bool user  = md.mode() & S_IXUSR;
	bool group = md.mode() & S_IXGRP;
	bool other = md.mode() & S_IXOTH;

	std::vector<posixok::Metadata_ReachabilityEntry> v;
	posixok::Metadata_ReachabilityEntry e;
	e.set_uid(md.id_user());
	e.set_gid(md.id_group());
	auto addEntry = [&v,&e](posixok::Metadata_ReachabilityType type) -> void {
		e.set_type(type);
		v.push_back(e);
		assert(v.size() <= 2);
	};

	/* Check if access is restricted to a specific user and / or group */
	if(!other){
		if ( user &&  group)	addEntry(md.UID_OR_GID);
		if ( user && !group) 	addEntry(md.UID);
	    if (!user &&  group)	addEntry(md.GID);
	}

	/* Check if a specific user and / or group is excluded. */
	if(!user) 				addEntry(md.NOT_UID);
	if(!group){
		if(!user) 			addEntry(md.NOT_GID);
		if( user && other)	addEntry(md.GID_REQ_UID);  // owner can access despite excluded group
	}

	auto equal = [](const posixok::Metadata_ReachabilityEntry &lhs, const posixok::Metadata_ReachabilityEntry &rhs) -> bool {
		if(lhs.uid()  != rhs.uid()) return false;
		if(lhs.gid()  != rhs.gid()) return false;
		if(lhs.type() != rhs.type()) return false;
		return true;
	};

	/* Remove all entries that are duplicates of entries stored in pathPermission.
	 * If a restriction is already enforced by a parent directory, there's no need to enforce it again. */
	for(int i=0; i < md.path_permission_size(); i++)
		for (auto it = v.begin(); it != v.end(); it++)
				if(equal(md.path_permission(i), *it))
					v.erase(it);


	/* Check if path_permission_children changed, if yes store in md. */
	bool changed = false;
	for(int i=0; i < md.path_permission_children_size(); i++)
		for (auto it = v.begin(); it != v.end(); it++)
				if(equal(md.path_permission_children(i), *it))
					changed = true;
	if(changed){
		md.mutable_path_permission_children()->Clear();
		for (auto it = v.begin(); it != v.end(); it++){
			posixok::Metadata_ReachabilityEntry *e = md.mutable_path_permission_children()->Add();
			e->set_gid((*it).gid());
			e->set_uid((*it).uid());
			e->set_type((*it).type());
		}
	}
	return changed;
}
