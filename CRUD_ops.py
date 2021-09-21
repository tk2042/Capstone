# !/usr/bin/env python3
# -*- coding: utf-8 -*-
# Timothy Kelly
# CS340 4-1 Milestone 
# CRUD operations

"""
Created on Wed Jan 27 10:51:31 2021

@author: iceliger
"""

from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations for Animal collection in MongoDB """
    
    def __init__(self, username, password):
        # Initialize the MongoClient. This helps to
        # access the MongoDB databases and collections.
        self.client = MongoClient('mongodb://%s:%s@127.0.0.1:29940/AAC' % (username, password))
        self.database = self.client['AAC']
            
    def create(self, data):
        # Checks to see if the data is null or empty and returns false in either case
        # Creation of C (create) operation.
        if data is not None:
                self.database.animals.insert_one(data)
                return True
        else:
            return False
        
    def read(self, search):
        # Checks to see if the data is null or empty and returns exceptions in either case
        # Creation of R (read) operation.
        if search is not None:
                searchResult = self.database.animals.find(search)
                return searchResult
        else:
            exception = "Nothing to search, because search parameter is empty"
            return exception
        
    def update(self, data, newData):
        # Creation of U (update) operation.
        if newData is not None:
            self.database.animals.update_one(data.get_as_json()) 
            return True
        else:
            return Exception("Nothing to update, because project parameter is None")
            
    def delete(self, data):
        # Creation of D (delete) operation.
        if data is not None:
            self.database.animals.delete_one(data.get_as_json())
            return True
        else:
            return Exception("Nothing to delete, because project parameter is None")
            
        
    