#pragma once
#include <Gaudi/Algorithm.h>

#include <string>
#include <vector>

namespace Gaudi
{
  class GAUDI_API Sequence : public Algorithm
  {
  public:
    using Algorithm::Algorithm;

    /// Are we a Sequence?
    bool isSequence() const override final { return true; }

    /** Create a sub algorithm.
     *  A call to this method creates a child algorithm object.
     *  Note that the returned pointer is to Algorithm
     *  (as opposed to IAlgorithm), and thus the methods of IProperty
     *  are also available for the direct setting of the sub-algorithm's
     *  properties. Using this mechanism instead of creating daughter
     *  algorithms directly via the new operator is prefered since then
     *  the framework may take care of all of the necessary book-keeping.
     *  @param type The concrete algorithm class of the sub algorithm
     *  @param name The name to be given to the sub algorithm
     *  @param pSubAlg Set to point to the newly created algorithm object
     */
    StatusCode createSubAlgorithm( const std::string& type, const std::string& name, Algorithm*& pSubAlg );

    /// List of sub-algorithms. Returns a pointer to a vector of (sub) Algorithms
    const std::vector<Algorithm*>* subAlgorithms() const;

    /// List of sub-algorithms. Returns a pointer to a vector of (sub) Algorithms
    std::vector<Algorithm*>* subAlgorithms();

    /** System start. This method invokes the start() method of a concrete
        algorithm and the start() methods of all of that algorithm's sub algorithms.
    */
    StatusCode start() override;

    /** Initialization method invoked by the framework. This method is responsible
     *  for any bookkeeping of initialization required by the framework itself.
     *  It will in turn invoke the initialize() method of the derived algorithm,
     * and of any sub-algorithms which it creates.
     */
    StatusCode initialize() override;

    /** Reinitialization method invoked by the framework. This method is responsible
     *  for any reinitialization required by the framework itself.
     *  It will in turn invoke the reinitialize() method of the derived algorithm,
     * and of any sub-algorithms which it creates.
     */
    StatusCode reinitialize() override;

    /** Restart method invoked by the framework.
        It will in turn invoke the restart() method of the derived algorithm,
        and of any sub-algorithms which it creates.
    */
    StatusCode restart() override;

    /** System stop. This method invokes the stop() method of a concrete
        algorithm and the stop() methods of all of that algorithm's sub algorithms.
    */
    StatusCode stop() override;

    /** System finalization. This method invokes the finalize() method of a
     *  concrete algorithm and the finalize() methods of all of that algorithm's
     *  sub algorithms.
     */
    StatusCode finalize() override;

    /** beginRun method invoked by the framework. This method is responsible
        for any beginRun actions required by the framework itself.
        It will in turn invoke the beginRun() method of the derived algorithm,
        and of any sub-algorithms which it creates.
    */
    StatusCode beginRun() override;

    /** endRun method invoked by the framework. This method is responsible
        for any endRun actions required by the framework itself.
        It will in turn invoke the endRun() method of the derived algorithm,
        and of any sub-algorithms which it creates.
    */
    StatusCode endRun() override;

    void acceptDHVisitor( IDataHandleVisitor* ) const override;

  private:
    std::vector<Algorithm*> m_subAlgms; ///< Sub algorithms
  };
}
