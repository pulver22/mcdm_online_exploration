/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifndef CRITERION_H
#define CRITERION_H

class Criterion
{
    public:
	//Constructor and destructor
	Criterion();
	Criterion(const String &name, double weight);
	~Criterion();
	
	//Other methods
	double evaluate(int x, int y, double[] map);
	double getEvaluation(Point p) const;
	
	//Setters and getters
	const String &getName() const;
	double getWeight() const;
	void setName(const QString &name);
	void setWeight(double weight);
  
  protected:
	String name;
	double weight;
	
  private:
	Hash<Point *, double> evaluation;
};

#endif // CRITERION_H