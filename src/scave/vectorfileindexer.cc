//=========================================================================
//  VECTORFILEINDEXER.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <sys/stat.h>
#include <errno.h>
#include <sstream>
#include <ostream>
#include "platmisc.h"
#include "stringutil.h"
#include "resultfilemanager.h"
#include "nodetype.h"
#include "nodetyperegistry.h"
#include "dataflowmanager.h"
#include "vectorfilereader.h"
#include "indexedvectorfile.h"
#include "indexfile.h"
#include "vectorfileindexer.h"

static inline bool existsFile(const std::string fileName)
{
    struct stat s;
    return stat(fileName.c_str(), &s)==0;
}

static std::string createTempFileName(const std::string baseFileName)
{
    std::string prefix = baseFileName;
    prefix.append(".temp");
    std::string tmpFileName = prefix;
    int serial = 0;

    while (existsFile(tmpFileName))
        tmpFileName = opp_stringf("%s%d", prefix.c_str(), serial++);
    return tmpFileName;
}

void VectorFileIndexer::generateIndex(const char* fileName)
{
    // load file
    ResultFileManager resultFileManager;
    ResultFile *f = resultFileManager.loadFile(fileName); // TODO: limit number of lines read
    if (!f)
    {
        throw opp_runtime_error("Error: %s: load() returned null", fileName);
    }
    else if (f->numUnrecognizedLines>0)
    {
        fprintf(stderr, "WARNING: %s: %d invalid/incomplete lines out of %d\n", fileName, f->numUnrecognizedLines, f->numLines);
    }

    RunList runs = resultFileManager.getRunsInFile(f);
    if (runs.size() != 1)
    {
        if (runs.size() == 0)
            fprintf(stderr, "WARNING: %s: contains no runs\n", fileName);
        else
            fprintf(stderr, "WARNING: %s: contains %d runs, expected 1\n", fileName, (int)runs.size());
        return;
    }

    Run *runRef = runs[0];

    //
    // assemble dataflow network for vectors
    //
    DataflowManager *dataflowManager = new DataflowManager;

    // create filereader node
    NodeType *readerNodeType=NodeTypeRegistry::instance()->getNodeType("vectorfilereader");
    StringMap attrs;
    attrs["filename"] = fileName;
    VectorFileReaderNode *reader = (VectorFileReaderNode*)readerNodeType->create(dataflowManager, attrs);

    // create filewriter node
    NodeType *writerNodeType=NodeTypeRegistry::instance()->getNodeType("indexedvectorfilewriter");
    std::string tmpFileName=createTempFileName(fileName);
    std::string indexFileName=IndexFile::getIndexFileName(fileName);
    std::string tmpIndexFileName=createTempFileName(indexFileName);
    attrs.clear();
    attrs["fileheader"]= generateHeader(runRef);
    attrs["blocksize"]="65536";
    attrs["filename"]=tmpFileName;
    attrs["indexfilename"]=tmpIndexFileName;
    IndexedVectorFileWriterNode *writer = (IndexedVectorFileWriterNode*)writerNodeType->create(dataflowManager, attrs);
    writer->setRun(runRef->runName.c_str(), runRef->attributes, runRef->moduleParams);

    // create a ports for each vector on reader node and writer node and connect them
    IDList vectorIDList = resultFileManager.getAllVectors();
    for (int i=0; i<(int)vectorIDList.size(); i++)
    {
         const VectorResult& vector = resultFileManager.getVector(vectorIDList.get(i));
         Port *readerPort = reader->addVector(vector);
         Port *writerPort = writer->addVector(vector);
         dataflowManager->connect(readerPort, writerPort);
    }

    // run!
    try
    {
        dataflowManager->execute();
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "Exception during indexing: %s\n", e.what());
        if (unlink(tmpFileName.c_str())!=0 && errno!=ENOENT)
            fprintf(stderr, "Cannot remove temporary file: %s\n", tmpFileName.c_str());
        if (unlink(tmpIndexFileName.c_str())!=0 && errno!=ENOENT)
            fprintf(stderr, "Cannot remove temporary index file: %s\n", tmpIndexFileName.c_str());
        delete dataflowManager;
        throw e;
    }

    delete dataflowManager;

    // rename vector file and index file
    if (unlink(fileName)!=0 && errno!=ENOENT)
        throw opp_runtime_error("Cannot remove original vector file `%s': %s", fileName, strerror(errno));
    if (unlink(indexFileName.c_str())!=0 && errno!=ENOENT)
        throw opp_runtime_error("Cannot remove original index file `%s': %s", indexFileName, strerror(errno));
    if (rename(tmpIndexFileName.c_str(), indexFileName.c_str())!=0)
        throw opp_runtime_error("Cannot rename index file from '%s' to '%s': %s", tmpIndexFileName.c_str(), indexFileName.c_str(), strerror(errno));
    if (rename(tmpFileName.c_str(), fileName)!=0)
        throw opp_runtime_error("Cannot rename vector file from '%s' to '%s': %s", tmpFileName.c_str(), fileName, strerror(errno));
}

std::string VectorFileIndexer::generateHeader(Run *runRef)
{
    std::stringstream header;

    header << "# generated by scave\n";
    if (runRef->runName.size() > 0)
        header << "run " << runRef->runName << "\n";

    for (StringMap::iterator attrRef = runRef->attributes.begin(); attrRef != runRef->attributes.end(); ++attrRef)
    {
        header << "attr " << attrRef->first << " " << QUOTE(attrRef->second.c_str()) << "\n";
    }

    for (StringMap::iterator paramRef = runRef->moduleParams.begin(); paramRef != runRef->moduleParams.end(); ++paramRef)
    {
        header << "param " << paramRef->first << " " << QUOTE(paramRef->second.c_str()) << "\n";
    }

    return header.str();
}
