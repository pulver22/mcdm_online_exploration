#include "mcdmfunction.h"
#include "explorationconstants.h"
#include "Criteria//criteriaName.h"
#include "Criteria/traveldistancecriterion.h"
#include "Criteria/informationgaincriterion.h"
#include "Criteria/sensingtimecriterion.h"
#include "Criteria/mcdmweightreader.h"
#include "Criteria/criterioncomparator.h"
#include <string>
#include <iostream>


/* create a list of criteria with name and <encoded_name,weight> pair after reading that from a file
 */
MCDMFunction::MCDMFunction() :
     criteria(new unordered_map<string, Criterion *>()),activeCriteria(NULL)
    
{
    //read the weight from somewhere
    MCDMWeightReader weightReader;
    matrix = weightReader.parseFile();
    // get the list of all criteria to be considered
    list<string> listCriteria = matrix->getKnownCriteria();
    list< string >::iterator l_front = listCriteria.begin();
    for (l_front; l_front != listCriteria.end(); ++l_front){
	string name = *l_front;
	// retrieve the weight of the criterion using the encoded version of the name
	double weight = matrix->getWeight(matrix->getNameEncoding(name));
	Criterion *c = createCriterion(name, weight);
	if(c != NULL)
	    criteria->insert(name, c);
    }
    

    //        QHash<QString, double> * configuration = weightReader.parseFile();
    //        foreach(QString crit, configuration->keys()){
    //            if(crit.contains("_")){
    //                jointCriteria.insert(crit, configuration->value(crit));
    //            } else {
    //                Criterion *criterion = createCriterion(crit, configuration->value(crit));
    //                if(criterion!=NULL){
    //                    criteria->append(criterion);
    //                }
    //            }
    //        }
    //        delete configuration;

}

MCDMFunction::~MCDMFunction()
{
    delete matrix;
    for (int i= criteria->size()-1; i >=0; i--){
	criteria->erase(i);
    }
    delete criteria;

}


Criterion * MCDMFunction::createCriterion(string name, double weight)
{
    Criterion *toRet = NULL;
    if(name == string(SENSING_TIME)){
	toRet = new SensingTimeCriterion(weight);
    } else if (name == string(INFORMATION_GAIN)) {
	toRet = new InformationGainCriterion(weight);
    } else if (name == string(TRAVEL_DISTANCE)){
	toRet = new TravelDistanceCriterion(weight);
    }
    return toRet;
}


double MCDMFunction::evaluateFrontier( const Pose *p, const Map &map)
{
    //Should keep the ordering of the criteria and the weight of each criteria combinations
   for (vector<Criterion *>::iterator it = *activeCriteria.begin(); it != activeCriteria.end(); it++){
       Criterion *c = *it;
       c->evaluate(p,map);
   }
   
    //for loop, over the criteria, to compute the utility of the frontier.

    return 0.0;
}



EvaluationRecords* MCDMFunction::evaluateFrontiers(const std::list< Pose* >& frontiers, const Map& map)
{         
    myMutex.lock();
   
    //Clean the last evaluation
    //         lprint << "clean evaluations" << endl;
    for(int i=0; i< criteria->size(); i++){
	criteria->clear(i);
    }

    //Get the list of activeCriteria
    if(activeCriteria == NULL){
	activeCriteria = new vector<Criterion *>();
    }
    vector<string> listActiveCriteria = matrix->getActiveCriteria();
    for(vector<string>::iterator it = listActiveCriteria.begin(); it != listActiveCriteria.end(); it++){
	activeCriteria.push_back(criteria[*it]);
    }
    
    //ldbg << "number of active criteria: "<<activeCriteria->size() << endl;

  
    //Evaluate the frontiers
    for (int i=0; i<frontiers.size(); i++){
	//Check if the frontier is reachable.
	Pose *f = frontiers[i];
//             Point posePoint(f->centroid().x(), f->centroid().y());
//             bool evaluate = PathPlanner::frontierFound(map.frontiers(), posePoint);
	double value = 0.0;
//             if(evaluate){
	    //The frontier is reachable => evaluate them
//             lprint << f->centroid().x() <<";"<<f->centroid().y()<<endl;
	value = evaluateFrontier(f, map);
//             } else {
//                 //The frontier is not reachable => set the worst value
//                 foreach(Criterion *c, *activeCriteria){
//                     c->setWorstValue(f);
//                 }
//             }
    }
    
    //Normalize the values
    for(vector<Criterion *>::iterator it = activeCriteria.begin(); it != activeCriteria.end(); ++it){
	(*it).normalize();
    }
    
    
    //Create the EvaluationRecords
    //         lprint << "#number of frontier to evaluate: "<<frontiers.size()<<endl;
    EvaluationRecords *toRet = new EvaluationRecords();
    
    for(list<Pose>::iterator i=frontiers.begin(); i!=frontiers.end(); i++){

	Pose *f = *i;
	qsort(activeCriteria,activeCriteria.size(),sizeof(Criterion),CriterionComparator(f));
	//qSort(activeCriteria->begin(), activeCriteria->end(), CriterionComparator(f));
	
	//apply the choquet integral
	Criterion *lastCrit = NULL;
	double finalValue = 0.0;
//             lprint << i << ") ";
//             Point myPoint(map.lastRobotPose(Config::robotID)->x(), map.lastRobotPose(Config::robotID)->y());
//             Point centroid = f->centroid();
//             if(myPoint.distance(centroid) < LASER_RANGE){
	for(vector<Criterion *>::iterator i = activeCriteria.begin(); i != activeCriteria.end(); i++){
	    Criterion *c = NULL;
	    double weight = 0.0;
	    //Get the list of criterion that are >= than the one considered
	    vector<string> names;
	   for(vector<Criterion *>::iterator j = i+1; j != activeCriteria.end(); j++){
	       //CHECK IF THE ITERATOR RETURN THE COUPLE <STRING,CRITERION>
	       Criterion *c = (*j);
	       names.push_back(c->getName());
	   }
	    weight = matrix->getWeight(names);
	
//               lprint << "#"<<names << " - w = " << weight << " - ";
//               lprint << names <<" with weight "<<weight<<endl;
	    if(i==activeCriteria.begin()){
		c = (*i);
		finalValue += c->getEvaluation(f) * weight;
//                         lprint << "#crit "<<c->getName()<<" - eval = "<<c->getEvaluation(f) << endl;
	    } else {
		c = (*i);
		double tmpValue = c->getEvaluation(f)-lastCrit->getEvaluation(f);
//                         lprint << "#crit "<<c->getName()<<" - eval = "<<c->getEvaluation(f) << endl;
		finalValue += tmpValue*weight;
	    }
	    lastCrit = c;
	}
	cout << f->getX() <<";"<<f->getY();
	cout <<";"<<finalValue << ";"<< endl;
	toRet->putEvaluation(*f, finalValue);
    }
//         }
    cout << endl;
    
    delete activeCriteria;
    activeCriteria = NULL;
    myMutex.unlock();
    return toRet;
}


