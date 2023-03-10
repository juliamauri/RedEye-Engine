#include "RE_Directory.hpp"

#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "RE_Shader.h"

#include "RE_ThreadPool.hpp"
#include "RE_Thread.h"

#include "RE_Config.h"
#include "RE_Json.h"

#include <PhysFS/physfs.h>

namespace RE_Directory {

	RE_File* RE_Path::AsFile() { return reinterpret_cast<RE_File*>(this); }
	RE_Meta* RE_Path::AsMeta() { return reinterpret_cast<RE_Meta*>(this); }
	RE_Directory* RE_Path::AsDirectory() { return reinterpret_cast<RE_Directory*>(this); }

	FileType RE_File::DetectExtensionAndType(const char* _path, const char*& _extension)
	{
		static const char* extensionsSuported[14] = { "meta", "re","refab", "pupil", "sk",  "fbx", "jpg", "dds", "png", "tga", "tiff", "bmp", "lasse", "lopfe" };

		FileType ret = FileType::F_NOTSUPPORTED;
		eastl::string modPath(_path);
		eastl::string filename = modPath.substr(modPath.find_last_of("/") + 1);
		eastl::string extensionStr = filename.substr(filename.find_last_of(".") + 1);
		eastl::transform(extensionStr.begin(), extensionStr.end(), extensionStr.begin(), [](unsigned char c) { return eastl::CharToLower(c); });

		for (unsigned int i = 0; i < 14; i++)
		{
			if (extensionStr.compare(extensionsSuported[i]) == 0)
			{
				_extension = extensionsSuported[i];
				switch (i) {
				case 0: ret = FileType::F_META; break;
				case 1: ret = FileType::F_SCENE; break;
				case 2: ret = FileType::F_PREFAB; break;
				case 3: ret = FileType::F_MATERIAL; break;
				case 4: ret = FileType::F_SKYBOX; break;
				case 5: ret = FileType::F_MODEL; break;
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11: ret = FileType::F_TEXTURE; break;
				case 12: ret = FileType::F_PARTICLEEMISSOR; break;
				case 13: ret = FileType::F_PARTICLERENDER; break;
				}
			}
		}
		return ret;
	}

	RE_Path* RE_File::AsPath() { return dynamic_cast<RE_Path*>(this); }
	RE_Meta* RE_File::AsMeta() { return reinterpret_cast<RE_Meta*>(this); }

	bool RE_Meta::IsModified() const
	{
		return (RE_RES->At(resource)->GetType() != Resource_Type::R_SHADER
			&& fromFile->lastModified != RE_RES->At(resource)->GetLastTimeModified());
	}

	RE_Path* RE_Meta::AsPath() { return dynamic_cast<RE_Path*>(this); }
	RE_File* RE_Meta::AsFile() { return dynamic_cast<RE_File*>(this); }

	void RE_Directory::AddBeforeOf(RE_Path* toAdd, eastl::list<RE_Path*>::iterator to) { tree.insert(to, toAdd); }
	void RE_Directory::Delete(eastl::list<RE_Path*>::iterator del) { tree.erase(del); }


	void RE_Directory::SetPath(const char* _path)
	{
		path = _path;
		name = eastl::string(path.c_str(), path.size() - 1);
		name = name.substr(name.find_last_of("/") + 1);
	}

	eastl::list<RE_Directory*> RE_Directory::MountTreeFolders()
	{
		eastl::list< RE_Directory*> ret;
		if (RE_FS->Exists(path.c_str()))
		{
			eastl::string iterPath(path);
			char** rc = PHYSFS_enumerateFiles(iterPath.c_str());
			for (char** i = rc; *i != NULL; i++)
			{
				eastl::string inPath(iterPath);
				inPath += *i;

				PHYSFS_Stat fileStat;
				if (PHYSFS_stat(inPath.c_str(), &fileStat) && fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
				{
					RE_Directory* newDirectory = new RE_Directory();
					newDirectory->SetPath((inPath += "/").c_str());
					newDirectory->parent = this;
					newDirectory->pType = PathType::D_FOLDER;
					tree.push_back(newDirectory->AsPath());
					ret.push_back(newDirectory);
				}
			}
			PHYSFS_freeList(rc);

			if (!tree.empty())
			{
				for (RE_Path* path : tree)
				{
					eastl::list< RE_Directory*> fromChild = path->AsDirectory()->MountTreeFolders();
					if (!fromChild.empty()) ret.insert(ret.end(), fromChild.begin(), fromChild.end());
				}
			}
		}
		return ret;
	}

	eastl::stack<RE_ProcessPath*> RE_Directory::CheckAndApply(eastl::vector<RE_Meta*>* metaRecentlyAdded)
	{
		eastl::stack<RE_ProcessPath*> ret;
		if (RE_FS->Exists(path.c_str()))
		{
			eastl::string iterPath(path);
			eastl::vector<RE_Meta*> toRemoveM;
			char** rc = PHYSFS_enumerateFiles(iterPath.c_str());
			auto iter = tree.begin();
			for (char** i = rc; *i != NULL; i++)
			{
				PathType iterTreeType = (iter != tree.end()) ? (*iter)->pType : PathType::D_NULL;
				eastl::string inPath(iterPath);
				inPath += *i;
				PHYSFS_Stat fileStat;
				if (PHYSFS_stat(inPath.c_str(), &fileStat))
				{
					if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY)
					{
						bool newFolder = (iterTreeType == PathType::D_NULL || iterTreeType != PathType::D_FOLDER || (*iter)->path != (inPath += "/"));
						if (newFolder && iter != tree.end())
						{
							auto postPath = iter;
							bool found = false;
							while (postPath != tree.end())
							{
								if (inPath == (*postPath)->path)
								{
									found = true;
									break;
								}
								postPath++;
							}
							if (found) newFolder = false;
						}

						if (newFolder)
						{
							RE_Directory* newDirectory = new RE_Directory();

							if (inPath.back() != '/')inPath += "/";
							newDirectory->SetPath(inPath.c_str());
							newDirectory->parent = this;
							newDirectory->pType = PathType::D_FOLDER;

							AddBeforeOf(newDirectory->AsPath(), iter);
							iter--;

							RE_ProcessPath* newProcess = new RE_ProcessPath();
							newProcess->procedure = PathProcessType::P_ADDFOLDER;
							newProcess->toProcess = newDirectory->AsPath();
							ret.push(newProcess);
						}
					}
					else if (fileStat.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_REGULAR)
					{
						const char* extension = nullptr;
						FileType fileType = RE_File::DetectExtensionAndType(inPath.c_str(), extension);
						bool newFile = (iterTreeType == PathType::D_NULL || iterTreeType != PathType::D_FILE || (*iter)->path != inPath);
						if (newFile && fileType == FileType::F_META && !metaRecentlyAdded->empty())
						{
							int sizemetap = 0;
							for (RE_Meta* metaAdded : *metaRecentlyAdded)
							{
								sizemetap = eastl::CharStrlen(metaAdded->path.c_str());
								if (sizemetap > 0 && eastl::Compare(metaAdded->path.c_str(), inPath.c_str(), sizemetap) == 0)
								{
									AddBeforeOf(metaAdded->AsPath(), iter);
									iter--;
									toRemoveM.push_back(metaAdded);
									metaRecentlyAdded->erase(eastl::remove_if(eastl::begin(*metaRecentlyAdded), eastl::end(*metaRecentlyAdded),
										[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(*metaRecentlyAdded));
									toRemoveM.clear();
									newFile = false;
									break;
								}
							}
						}

						if (newFile && iter != tree.end())
						{
							auto postPath = iter;
							bool found = false;
							while (postPath != tree.end())
							{
								if (inPath == (*postPath)->path)
								{
									found = true;
									break;
								}
								postPath++;
							}
							if (found) newFile = false;
						}

						if (newFile)
						{
							RE_File* newFile = (fileType != FileType::F_META) ? new RE_File() : (RE_File*)new RE_Meta();
							newFile->path = inPath;
							newFile->filename = inPath.substr(inPath.find_last_of("/") + 1);
							newFile->lastSize = fileStat.filesize;
							newFile->pType = PathType::D_FILE;
							newFile->fType = fileType;
							newFile->extension = extension;
							newFile->lastModified = fileStat.modtime;

							RE_ProcessPath* newProcess = new RE_ProcessPath();
							newProcess->procedure = PathProcessType::P_ADDFILE;
							newProcess->toProcess = newFile->AsPath();
							ret.push(newProcess);
							AddBeforeOf(newFile->AsPath(), iter);
							iter--;
						}
						else if ((*iter)->AsFile()->fType == FileType::F_META)
						{
							bool reimport = false;
							const ResourceContainer* res = RE_RES->At((*iter)->AsFile()->AsMeta()->resource);
							switch (res->GetType())
							{
							case R_SHADER: {
								const RE_Shader* shadeRes = dynamic_cast<const RE_Shader*>(res);
								if (shadeRes->isShaderFilesChanged())
								{
									RE_ProcessPath* newProcess = new RE_ProcessPath();
									newProcess->procedure = PathProcessType::P_REIMPORT;
									newProcess->toProcess = (*iter);
									ret.push(newProcess);
								}
								break; }
							case R_PARTICLE_RENDER:
							case R_PARTICLE_EMISSION: {

								if ((*iter)->AsFile()->lastModified != fileStat.modtime) {
									(*iter)->AsFile()->lastModified = fileStat.modtime;

									RE_ProcessPath* newProcess = new RE_ProcessPath();
									newProcess->procedure = PathProcessType::P_REIMPORT;
									newProcess->toProcess = (*iter);
									ret.push(newProcess);
								}
								break;
							}
							}
						}
					}
				}
				if (iter != tree.end()) iter++;
			}
			PHYSFS_freeList(rc);
		}
		return ret;
	}

	eastl::stack<RE_Path*> RE_Directory::GetDisplayingFiles() const
	{
		eastl::stack<RE_Path*> ret;

		for (auto path : tree)
		{
			switch (path->pType)
			{
			case PathType::D_FILE:
			{
				switch (path->AsFile()->fType)
				{
				case FileType::F_META:
				{
					if (path->AsMeta()->resource)
					{
						const ResourceContainer* res = RE_RES->At(path->AsMeta()->resource);
						if (res->GetType() == R_SHADER || res->GetType() == R_PARTICLE_EMITTER) ret.push(path);
					}
					break;
				}
				case FileType::F_NONE: break;
				default: ret.push(path); break;
				}
				break;
			}
			case PathType::D_FOLDER: ret.push(path); break;
			}
		}

		return ret;
	}

	eastl::list<const RE_Directory*> RE_Directory::FromParentToThis() const
	{
		eastl::list<const RE_Directory*> ret;
		const RE_Directory* iter = this;
		while (iter != nullptr)
		{
			ret.push_front(iter);
			iter = iter->parent;
		}
		return ret;
	}

	RE_Path* RE_Directory::AsPath() { return dynamic_cast<RE_Path*>(this); }

	namespace Assets
	{
		namespace
		{
			EA::Thread::Mutex _const_assetsDirectories_mutex;
			eastl::list<const RE_Directory*> _const_assetsDirectories;
			eastl::list<RE_Directory*> assetsDirectories;

			EA::Thread::Mutex _toDelete_mutex;
			eastl::stack<eastl::string> _toDelete;

			eastl::list<RE_Directory*>::iterator dirIter;
			eastl::stack<RE_ProcessPath*> assets_to_process;
			eastl::stack<RE_ProcessPath*> meta_to_process_last;

			eastl::vector<RE_Meta*> metaToFindFile;
			eastl::stack<RE_Meta*> reloadResourceMeta;
			eastl::vector<RE_File*> filesToFindMeta;
			eastl::vector<RE_Meta*> metaRecentlyAdded;

			static RE_Directory dir;

			struct AssetsPrioroty {
				bool operator()(const RE_File* lhs, const RE_File* rhs) const
				{
					return lhs->fType > rhs->fType;
				}
			};

			eastl::priority_queue<RE_File*, eastl::vector<RE_File*>, AssetsPrioroty> toImport;
			eastl::priority_queue<RE_File*, eastl::vector<RE_File*>, AssetsPrioroty> toReImport;


			class ReadAssetsChanges : public RE_Thread
			{
			public:
				~ReadAssetsChanges()
				{
					for (auto dir : assetsDirectories)
						for (auto p : dir->tree)
							if (p->pType != PathType::D_FOLDER)
								delete p;

					assetsDirectories.pop_front();
					for (auto dir : assetsDirectories) delete dir;
				}
			private:
				void RunJob(void* pContext = NULL)
				{
					// Rubén TODO: Make PROFILE threaded
					//RE_PROFILE(PROF_ReadAssetChanges, PROF_FileSystem);

					while (!_toDelete.empty())
					{
						eastl::string filePath = _toDelete.top();
						_toDelete.pop();

						RE_Directory* dir = FindDirectory(filePath.c_str());

						// RE_LOG_ERROR("Error deleting file. The dir culdn't be located: %s", filePath);
						if (dir == nullptr) continue;

						RE_Path* file = FindPath(filePath.c_str(), dir);

						//RE_LOG_ERROR("Error deleting file. The file culdn't be located: %s", filePath);
						if (file == nullptr) continue;

						auto iterPath = dir->tree.begin();
						while (iterPath != dir->tree.end())
						{
							if (*iterPath == file)
							{
								dir->tree.erase(iterPath);
								break;
							}
							iterPath++;
						}

						delete file;
					}

					if ((dirIter == assetsDirectories.begin()) && (!assets_to_process.empty() || !filesToFindMeta.empty() || !toImport.empty() || !toReImport.empty()))
						dirIter = assetsDirectories.end();

					while (dirIter != assetsDirectories.end())
					{
						if (neededEnd()) return;
						eastl::stack<RE_ProcessPath*> toProcess = (*dirIter)->CheckAndApply(&metaRecentlyAdded);
						while (!toProcess.empty())
						{
							RE_ProcessPath* process = toProcess.top();
							toProcess.pop();
							assets_to_process.push(process);
						}
						dirIter++;
					}

					if (dirIter == assetsDirectories.end())
					{
						dirIter = assetsDirectories.begin();

						while (!assets_to_process.empty())
						{
							RE_ProcessPath* process = assets_to_process.top();
							assets_to_process.pop();

							switch (process->procedure)
							{
							case PathProcessType::P_ADDFILE:
							{
								RE_File* file = process->toProcess->AsFile();
								RE_Meta* metaFile = file->AsMeta();

								FileType type = file->fType;

								if (type == FileType::F_META)
								{
									const char* isReferenced = RE_RES->FindMD5ByMETAPath(file->path.c_str());

									if (!isReferenced)
									{
										Config metaLoad(file->path.c_str());
										if (metaLoad.Load())
										{
											RE_Json* metaNode = metaLoad.GetRootNode("meta");
											Resource_Type type = (Resource_Type)metaNode->PullInt("Type", Resource_Type::R_UNDEFINED);
											delete metaNode;

											if (type == Resource_Type::R_PARTICLE_EMITTER) {
												meta_to_process_last.push(process);
												continue;
											}

											if (RE_RES->isNeededResoursesLoaded(file->path.c_str(), type))
												reloadResourceMeta.push(metaFile);

											if (type != Resource_Type::R_UNDEFINED)
												metaFile->resource = RE_RES->ReferenceByMeta(file->path.c_str(), type);
										}
									}
									else
										metaFile->resource = isReferenced;

									metaToFindFile.push_back(metaFile);
								}
								else if (type > FileType::F_NONE) filesToFindMeta.push_back(file);
								else RE_LOG_ERROR("Unsupported file type");
								break;
							}
							case PathProcessType::P_ADDFOLDER:
							{
								RE_Directory* dir = (RE_Directory*)process->toProcess;
								eastl::stack<RE_ProcessPath*> toProcess = dir->CheckAndApply(&metaRecentlyAdded);

								while (!toProcess.empty())
								{
									RE_ProcessPath* process = toProcess.top();
									toProcess.pop();
									assets_to_process.push(process);
								}

								assetsDirectories.push_back(dir);

								_const_assetsDirectories_mutex.Lock();
								_const_assetsDirectories.push_back(const_cast<const RE_Directory*>(dir));
								_const_assetsDirectories_mutex.Unlock();
								break;
							}
							case PathProcessType::P_REIMPORT:
							{
								toReImport.push(process->toProcess->AsFile());
								break;
							}
							}
							delete process;
						}

						while (!meta_to_process_last.empty())
						{
							RE_ProcessPath* process = meta_to_process_last.top();
							meta_to_process_last.pop();

							RE_File* file = process->toProcess->AsFile();
							RE_Meta* metaFile = file->AsMeta();

							metaFile->resource = RE_RES->ReferenceByMeta(file->path.c_str(), Resource_Type::R_PARTICLE_EMITTER);
						}

						if (!metaToFindFile.empty())
						{
							eastl::vector<RE_File*> toRemoveF;
							eastl::vector<RE_Meta*> toRemoveM;

							for (RE_Meta* meta : metaToFindFile)
							{
								const ResourceContainer* res = RE_RES->At(meta->resource);
								const char* assetPath = RE_RES->At(meta->resource)->GetAssetPath();

								if (!res->isInternal())
								{
									int sizefile = 0;
									if (!filesToFindMeta.empty())
									{
										for (RE_File* file : filesToFindMeta)
										{
											sizefile = eastl::CharStrlen(assetPath);
											if (sizefile > 0 && eastl::Compare(assetPath, file->path.c_str(), sizefile) == 0)
											{
												meta->fromFile = file;
												file->metaResource = meta;
												toRemoveF.push_back(file);
												toRemoveM.push_back(meta);
											}
										}
									}
									if (res->GetType() == R_SHADER)  toRemoveM.push_back(meta);
								}
								else toRemoveM.push_back(meta);
							}
							if (!toRemoveF.empty())
							{
								//https://stackoverflow.com/questions/21195217/elegant-way-to-remove-all-elements-of-a-vector-that-are-contained-in-another-vec
								filesToFindMeta.erase(eastl::remove_if(eastl::begin(filesToFindMeta), eastl::end(filesToFindMeta),
									[&](auto x) {return eastl::find(begin(toRemoveF), end(toRemoveF), x) != end(toRemoveF); }), eastl::end(filesToFindMeta));
							}
							if (!toRemoveM.empty())
							{
								metaToFindFile.erase(eastl::remove_if(eastl::begin(metaToFindFile), eastl::end(metaToFindFile),
									[&](auto x) {return eastl::find(begin(toRemoveM), end(toRemoveM), x) != end(toRemoveM); }), eastl::end(metaToFindFile));
							}
						}

						if (!filesToFindMeta.empty())
						{
							for (RE_File* file : filesToFindMeta)
							{
								toImport.push(file);
							}
							filesToFindMeta.clear();
						}

						if (!toImport.empty()) {
							RE_LOGGER.ScopeProcedureLogging();
							while (!toImport.empty())
							{

								RE_File* file = toImport.top();
								toImport.pop();

								//Importing
								RE_LOG("Importing %s", file->path.c_str());

								const char* newRes = nullptr;
								switch (file->fType) {
								case FileType::F_MODEL:	 newRes = RE_RES->ImportModel(file->path.c_str()); break;
								case FileType::F_TEXTURE:	 newRes = RE_RES->ImportTexture(file->path.c_str()); break;
								case FileType::F_MATERIAL: newRes = RE_RES->ImportMaterial(file->path.c_str()); break;
								case FileType::F_SKYBOX:	 newRes = RE_RES->ImportSkyBox(file->path.c_str()); break;
								case FileType::F_PREFAB:	 newRes = RE_RES->ImportPrefab(file->path.c_str()); break;
								case FileType::F_SCENE:	 newRes = RE_RES->ImportScene(file->path.c_str()); break;
								case FileType::F_PARTICLEEMISSOR:	 newRes = RE_RES->ImportParticleEmissor(file->path.c_str()); break;
								case FileType::F_PARTICLERENDER:	 newRes = RE_RES->ImportParticleRender(file->path.c_str()); break;
								}

								if (newRes != nullptr)
								{
									RE_Meta* newMetaFile = new RE_Meta();
									newMetaFile->resource = newRes;
									newMetaFile->fromFile = file;
									file->metaResource = newMetaFile;
									RE_File* fromMetaF = newMetaFile->AsFile();
									fromMetaF->fType = FileType::F_META;
									fromMetaF->path = RE_RES->At(newRes)->GetMetaPath();
									newMetaFile->AsPath()->pType = PathType::D_FILE;
									metaRecentlyAdded.push_back(newMetaFile);
								}
								else
									file->fType = FileType::F_NOTSUPPORTED;
							}
							RE_LOGGER.EndScope();
						}

						while (!reloadResourceMeta.empty())
						{
							RE_Meta* metaFile = reloadResourceMeta.top();
							reloadResourceMeta.pop();

							RE_RES->LoadResourceMeta(metaFile->resource);
						}

						if (!toReImport.empty())
						{
							static bool particle_reimport = false;
							while (!toReImport.empty())
							{
								RE_Meta* meta = toReImport.top()->AsMeta();
								toReImport.pop();

								RE_LOG("ReImporting %s", meta->path.c_str());

								switch (RE_RES->At(meta->resource)->GetType())
								{
								case R_SHADER:
									RE_RES->ReImportResource(meta->resource);
									break;
								case R_PARTICLE_RENDER:
								case R_PARTICLE_EMISSION:
									RE_RES->PushParticleResource(meta->resource);
									particle_reimport = true;
									break;
								}
							}
							if (particle_reimport) {
								RE_RES->ProcessParticlesReimport();
								particle_reimport = false;
							}
						}
					}
				}
			} changes;
		}

		void Init()
		{
			dir.SetPath("Assets/");
			assetsDirectories = dir.MountTreeFolders();
			assetsDirectories.push_front(&dir);
			dirIter = assetsDirectories.begin();

			_const_assetsDirectories_mutex.Lock();
			for (auto dir : assetsDirectories) _const_assetsDirectories.push_back(const_cast<const RE_Directory*>(dir));
			_const_assetsDirectories_mutex.Unlock();

			RE_ThreadPool::AddThread(dynamic_cast<RE_Thread*>(&changes));
		}

		void CleanUp() { changes.Finish(); }

		RE_Directory* FindDirectory(const char* pathToFind)
		{
			RE_Directory* ret = nullptr;

			eastl::string fullpath(pathToFind);
			eastl::string directoryStr = fullpath.substr(0, fullpath.find_last_of('/') + 1);

			for (auto dir : assetsDirectories)
			{
				if (dir->path == directoryStr)
				{
					ret = dir;
					break;
				}
			}
			return ret;
		}

		RE_Path* FindPath(const char* pathToFind, RE_Directory* dir)
		{
			RE_Path* ret = nullptr;

			eastl::string fullpath(pathToFind);
			eastl::string directoryStr = fullpath.substr(0, fullpath.find_last_of('/') + 1);

			if (!dir) dir = FindDirectory(pathToFind);
			if (dir)
			{
				for (auto file : dir->tree)
				{
					if (file->path == fullpath)
					{
						ret = file;
						break;
					}
				}
			}
			return ret;
		}

		eastl::list<const RE_Directory*> GetDirectories()
		{
			EA::Thread::AutoMutex am(_const_assetsDirectories_mutex);
			return _const_assetsDirectories;
		}

		const RE_Directory* GetRoot()
		{
			EA::Thread::AutoMutex am(_const_assetsDirectories_mutex);
			return *_const_assetsDirectories.begin();
		}

		void DeleteFile(const char* filepath)
		{
			EA::Thread::AutoMutex am(_toDelete_mutex);
			_toDelete.push(filepath);
		}
	}
}