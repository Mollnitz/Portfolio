# Data header -     UserID (f.key to User) | ItemID (f.key to Item) | Rating | TimeStamp 

# Item Header:      movie id | movie title | release date | video release date |
#                   IMDb URL | unknown | Action | Adventure | Animation |
#                   Children's | Comedy | Crime | Documentary | Drama | Fantasy |
#                   Film-Noir | Horror | Musical | Mystery | Romance | Sci-Fi |
#                   Thriller | War | Western |

# User Header -     UserID | Age | Gender | Occupation | Zip code

# - base files are 80k long approximately.
# - test files are 20k long.

import operator
import numpy as np
import math

#Makes it easy to access, even array indexes are base, uneven are test.
datafiles = ["u1.base", "u1.test", "u2.base", "u2.test", "u3.base", "u3.test", "u4.base", "u4.test", "u5.base", "u5.test"]
itemfile = "u.item"
userfile = "u.user"


seperatorline = "|" #oddly some files don't use "\t"
seperatortab = "\t"

def isint(val):
    try:
        int(val)
        return True
    except:
        return False

def file_to_dict(filename, cut_at_item = 0, seperator = seperatorline, index_limit = 0, key_index_mode = 0):
    res_dict = {}
    f = open(filename, "r")

    for index, line in enumerate(f.readlines()):
        items = line.split(seperator)

        #strings to integer values
        for index, item in enumerate(items):
            if isint(item):
                items[index] = int(item)
        

        #cut data off, useful to avoid date time
        if cut_at_item != 0:
            items = items[:cut_at_item]

        if key_index_mode == 0:
            res_dict[items[0]] = items[1:]
        else:
            res_dict[tuple(items[:key_index_mode])] = int(items[key_index_mode:][0]) 
        if index_limit != 0 and index_limit == index:
            break
    return res_dict


def calculate_averages(rating_dict):
    movies = {}
    users = {}

    total = 0
    for index, item in enumerate(rating_dict.keys()):
        userid = item[0]
        movieid = item[1]
        total += rating_dict[item]
        
        #Wet code, create function
        if userid in users:
            updateval = tuple(map(operator.add, users[userid], (rating_dict[item], 1)))
            users[userid] = updateval
        else:
            users[userid] = (rating_dict[item], 1)

        if movieid in movies:
            updateval = tuple(map(operator.add, movies[movieid], (rating_dict[item], 1)))
            movies[movieid] = updateval
        else:
            movies[movieid] = (rating_dict[item], 1)

    total = total / len(rating_dict)

    #Averaging - create function
    for item in movies.keys():
        movies[item] = movies[item][0] / movies[item][1]
    
    #Averaging.
    for item in users.keys():
        users[item] = users[item][0] / users[item][1] 

    return total, movies, users

def preprocess_values(values, total, movies, users):
    for item in values.keys():
        values[item] = values[item] - users[item[0]] - movies[item[1]] + total

def get_movie_genre_values(movie_item):
    return movie_item[4:]

#Amount = items, factors = factor pr item, value = initial value.
def latent_value_initializer(amount, factors = 17, movie = True):
    if movie:
        return np.random.rand(amount,factors)
    else: 
        return np.random.rand(factors, amount)

def add_general_factors(movie_general_factors):
    return 0

def predict(movie_latent_values, user_latent_values, movie, user, movie_avg = 0, user_avg = 0, base = 0):
    generalized = np.dot(movie_latent_values[movie -1,:], user_latent_values[:,user -1])
    #TODO: add in general factors
    generalized -= base
    generalized += movie_avg + user_avg
    return generalized

def main():
    #dict creation
    users = file_to_dict(userfile)
    items = file_to_dict(itemfile)

    #c) Construct a matrix factorization CF model (a.k.a. “Funk-SVD”) for this training data. Use between 10 and 50 latent factors. 
    user_latent_values = latent_value_initializer(len(users), movie = False, factors = 100) 
    movie_latent_values = latent_value_initializer(len(items), movie = True, factors = 100)

    #RMSE's
    # 10 x 5 runs med 10 factors: RMSE: 0.8022170561522384
    # 10 x 5 runs med 50 factors: RMSE: 0.689057330202062
    # 10 x 5 runs med 100 factors: RMSE 0.54
    for i in range(0,10):
        user_latent_values, movie_latent_values = train(user_latent_values, movie_latent_values, datafiles[0], datafiles[1], baselimit= 80000)
        user_latent_values, movie_latent_values = train(user_latent_values, movie_latent_values, datafiles[2], datafiles[3], baselimit= 80000)
        user_latent_values, movie_latent_values = train(user_latent_values, movie_latent_values, datafiles[4], datafiles[5], baselimit= 80000)
        user_latent_values, movie_latent_values = train(user_latent_values, movie_latent_values, datafiles[6], datafiles[7], baselimit= 80000)
        user_latent_values, movie_latent_values = train(user_latent_values, movie_latent_values, datafiles[8], datafiles[9], baselimit= 80000)
    


def train(user_latent_values, movie_latent_values, basepath, testpath, baselimit = 10000, testlimit = 200):
    #load base and test
    base1 = file_to_dict(basepath, 3, seperatortab, baselimit, 2)
    test1 = file_to_dict(testpath, 3, seperatortab, testlimit, 2)

    # Look up with base1[1,1] - base1[UserID, MovieID]
    
    #pre processing - b) Subtract the movie and user means from the training data (the pre-processing step from the slides).  
    total, movies_avg, users_avg = calculate_averages({**base1, **test1})
    
    #Make a copy of both the training and testing set 
    base1_unbiased = base1.copy()
    test1_unbiased = test1.copy()
    
    #Then, change their value to the averages.
    preprocess_values(base1_unbiased, total, movies_avg, users_avg)
    preprocess_values(test1_unbiased, total, movies_avg, users_avg)

    #prediction example

    #I slides er A movies og b er users - Am er en specifik movie column og Bu er en specifik user row.
    for index, item in enumerate(base1.keys()):
        n = 0.001
        unbiased_score = base1_unbiased[item] #R_mu

        #Splitting key.
        user_index = item[0]
        movie_index = item[1]

        movie_avg = movies_avg[movie_index]
        user_avg = users_avg[user_index]

        predict_val = predict(movie_latent_values, user_latent_values, movie_index, user_index)

        #Get the correct row and column
        user_column = user_latent_values[:,user_index -1] #B_ku 
        movie_row = movie_latent_values[movie_index -1, :] #A_mk

        #This is refering to slide 32 - I'm not sure why you want to multiply a column or row onto the value you are working with.
        mrow_update_vector = n * (unbiased_score - predict_val) * user_column 
        ucol_update_vector = n * movie_row * (unbiased_score - predict_val) 

        user_latent_values[:,user_index -1] = np.add(user_column, ucol_update_vector)
        movie_latent_values[movie_index -1, :] = np.add(movie_row, mrow_update_vector)


    #test
    miss_val = 0
    for index, item in enumerate(test1.keys()):
        real_score = test1[item] #R_mu

        user_index = item[0]
        movie_index = item[1]

        movie_avg = movies_avg[movie_index]
        user_avg = users_avg[user_index]

        predict_val = predict(movie_latent_values, user_latent_values, movie_index, user_index, movie_avg, user_avg, total)

        #RMSE 
        miss_val += (predict_val - real_score) ** 2

        #print("predicted: " + str(predict_val) + " real score was: " + str(real_score) + " Error is: " + str(real_score - predict_val) )


    miss_val /= len(test1)
    miss_val = math.sqrt(miss_val)
    print("RMSE: " + str(miss_val))
    return user_latent_values, movie_latent_values




    

if __name__ == "__main__":
    main()