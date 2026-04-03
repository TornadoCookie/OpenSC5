#include "Editor.hpp"

PackageLoader::PackageLoader()
    : mHasLoadedPkg(false),
      mIsLoadingPackage(false)
{}

void PackageLoader::LoadPackage(const char *packageFilename)
{
    FILE *f = fopen(packageFilename, "rb");

    if (!f)
    {
        perror(packageFilename);
        return;
    }

    if (mHasLoadedPkg)
    {
        UnloadPackageFile(mLoadedPkg);
        mEntries.clear();
    }

    mHasLoadedPkg = false;

    mPackageLoadState = {
        .f = f,
        .pkg = &mLoadedPkg,
        .done = false,
    };

    LoadPackageFileAsync(&mPackageLoadState);

    mIsLoadingPackage = true;
}

bool PackageLoader::HasLoadedPackage()
{
    return mHasLoadedPkg;
}

bool PackageLoader::IsLoadingPackage()
{
    return mIsLoadingPackage;
}

void PackageLoader::Tick()
{
    if (mIsLoadingPackage)
    {
        mIsLoadingPackage = !mPackageLoadState.done;
        if (mPackageLoadState.done)
        {
            mHasLoadedPkg = true;
            PopulateEntries();
            fclose(mPackageLoadState.f);
        }
    }
}

std::vector<PackageEntry *> PackageLoader::GetEntries(unsigned int type)
{
    return mEntries[type];
}

PackageEntry *PackageLoader::FindInstance(unsigned int instance)
{
    for (int i = 0; i < mLoadedPkg.entryCount; i++)
    {
        if (mLoadedPkg.entries[i].instance == instance)
            return &mLoadedPkg.entries[i];
    }

    return nullptr;
}

std::vector<unsigned int> PackageLoader::GetTypes()
{
    std::vector<unsigned int> out = {};

    for (std::pair<unsigned int, std::vector<PackageEntry *>> v : mEntries)
    {
        out.push_back(v.first);
    }

    return out;
}

static void LoadEntryData(PackageEntry *entry)
{
    switch (entry->type)
    {
        case PKGENTRY_PROP:
        {
            entry->data.propData = LoadPropData(entry->dataRaw, entry->dataRawSize);

            if (entry->data.propData.corrupted == true)
                ExportPackageEntry(*entry, NULL);
        } break;
        case PKGENTRY_ER2:
        case PKGENTRY_HTML:
        case PKGENTRY_CSS:
        case PKGENTRY_JSN8:
        case PKGENTRY_SCPT: // Script file format (?)
        case PKGENTRY_TEXT: // "textual" file.
        case PKGENTRY_JSON: // JSON file.
        {
            char *str = (char*)malloc(entry->dataRawSize);
            int actualSize = 0;

            // Filter out these disgusting UTF-8 characters.
            for (int i = 0; i < entry->dataRawSize; i++)
            {
                if (!isprint(entry->dataRaw[i]) && entry->dataRaw[i] != '\n' && entry->dataRaw[i] != '\t') continue;
                str[actualSize] = entry->dataRaw[i];
                actualSize++;
            }

            str = (char*)realloc(str, actualSize + 1);
            str[actualSize] = 0;
            entry->data.scriptSource = str;
        } break;
    }
}

void PackageLoader::PopulateEntries()
{
    for (int i = 0; i < mLoadedPkg.entryCount; i++)
    {
        PackageEntry *ent = &mLoadedPkg.entries[i];

        LoadEntryData(ent);

        mEntries[ent->type].push_back(ent);
    }
}
